// -*-c++-*-

/***************************************************************************
**                                                                        **
**  Controlling interface for K8090 8-Channel Relay Card from Velleman    **
**  through usb using virtual serial port in Qt.                          **
**  Copyright (C) 2018 Jakub Klener                                       **
**                                                                        **
**  This file is part of SpRelay application.                             **
**                                                                        **
**  You can redistribute it and/or modify it under the terms of the       **
**  3-Clause BSD License as published by the Open Source Initiative.      **
**                                                                        **
**  This program is distributed in the hope that it will be useful,       **
**  but WITHOUT ANY WARRANTY; without even the implied warranty of        **
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the          **
**  3-Clause BSD License for more details.                                **
**                                                                        **
**  You should have received a copy of the 3-Clause BSD License along     **
**  with this program.                                                    **
**  If not, see https://opensource.org/licenses/                          **
**                                                                        **
****************************************************************************/

/*!
 * \file      unified_serial_port_test.cpp
 * \brief     The biomolecules::sprelay::core::UnifiedSerialPortTest class which implements tests for
 *            biomolecules::sprelay::core::UnifiedSerialPort.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "unified_serial_port_test.h"

#include <QByteArray>
#include <QElapsedTimer>
#include <QEventLoop>
#include <QStringBuilder>
#include <QTimer>
#include <QtTest>

#include <algorithm>
#include <chrono>
#include <list>
#include <thread>
#include <utility>

#include "biomolecules/sprelay/core/k8090_commands.h"
#include "biomolecules/sprelay/core/k8090_utils.h"
#include "biomolecules/sprelay/core/serial_port_utils.h"
#include "biomolecules/sprelay/core/unified_serial_port.h"

#include "core_test_utils.h"

namespace biomolecules {
namespace sprelay {
namespace core {

const int UnifiedSerialPortTest::kCommandTimeoutMs = 50;
const int UnifiedSerialPortTest::kFactoryDefaultsTimeoutMs = 2 * UnifiedSerialPortTest::kCommandTimeoutMs;
const int UnifiedSerialPortTest::kDelayBetweenCommandsMs = 20;


void UnifiedSerialPortTest::initTestCase()
{
    real_card_present_ = false;
    foreach (const serial_utils::ComPortParams& params, UnifiedSerialPort::availablePorts()) {
        if (params.product_identifier == k8090::impl_::kProductID
            && params.vendor_identifier == k8090::impl_::kVendorID) {
            if (params.port_name != UnifiedSerialPort::kMockPortName) {
                real_card_port_name_ = params.port_name;
                real_card_present_ = true;
                break;
            }
        }
    }
}


void UnifiedSerialPortTest::availablePorts()
{
    // test, if the mock port is always present
    bool mock_found = false;
    foreach (const serial_utils::ComPortParams& params, UnifiedSerialPort::availablePorts()) {
        if (params.port_name == UnifiedSerialPort::kMockPortName
            && params.product_identifier == k8090::impl_::kProductID
            && params.vendor_identifier == k8090::impl_::kVendorID) {
            mock_found = true;
        }
    }
    QCOMPARE(mock_found, true);
}


void UnifiedSerialPortTest::switchRealVirtual()
{
    if (!real_card_present_) {
        QSKIP("Benchmark of real serial port needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }

    // create real port
    std::unique_ptr<UnifiedSerialPort> serial_port{new UnifiedSerialPort};
    serial_port->setPortName(real_card_port_name_);
    serial_port->setBaudRate(QSerialPort::Baud19200);
    serial_port->setDataBits(QSerialPort::Data8);
    serial_port->setParity(QSerialPort::NoParity);
    serial_port->setStopBits(QSerialPort::OneStop);
    serial_port->setFlowControl(QSerialPort::NoFlowControl);

    QVERIFY2(!serial_port->isOpen(), "The serial port shouldn't be oppened yet.");
    QVERIFY2(!serial_port->isReal(), "The serial port can be real or virtual only after it is opened.");
    QVERIFY2(!serial_port->isMock(), "The serial port can be real or virtual only after it is opened.");

    // open serial port
    if (!serial_port->open(QIODevice::ReadWrite)) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }
    QVERIFY2(serial_port->isOpen(), "The serial port should be oppened by now.");
    QVERIFY2(serial_port->isReal(), "The serial port should be real or virtual when it is opened.");
    resetRelays(serial_port.get());

    //                                                STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char on[] /*          */ = {0x04, 0x11, 0x01, 0x00, 0x00, 0xea, 0x0f};
    static const unsigned char response_on[] /* */ = {0x04, 0x51, 0x00, 0x01, 0x00, 0xaa, 0x0f};
    static const unsigned char query_status[] /**/ = {0x04, 0x18, 0x00, 0x00, 0x00, 0xe4, 0x0f};
    static const unsigned char response1[] /*   */ = {0x04, 0x51, 0x00, 0x00, 0x00, 0xab, 0x0f};
    static const unsigned char response2[] /*   */ = {0x04, 0x51, 0x01, 0x01, 0x00, 0xa9, 0x0f};

    // benchmark the command
    qint64 real_elapsed_time = 0;
    {  // real port
        qint64 local_elapsed_time;
        if (!measureCommandWithResponse(serial_port.get(), on, &local_elapsed_time)) {
            QFAIL("There is no response from the card.");
        }
        real_elapsed_time += local_elapsed_time;

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 but should have 7"}.arg(n)));
        }
        QVERIFY2(compareResponse(buffer, response_on),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(response_on, 7))));
    }

    // change to virtual port, the parameters should preserve and transfer to the newly oppened port
    serial_port->setPortName(UnifiedSerialPort::kMockPortName);
    QVERIFY2(serial_port->isOpen(), "The serial port should be still oppened now.");
    QVERIFY2(serial_port->isReal(), "The serial port should be still real now.");
    if (!serial_port->open(QIODevice::ReadWrite)) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(UnifiedSerialPort::kMockPortName)));
    }
    QVERIFY2(serial_port->isOpen(), "The serial port should be oppened by now.");
    QVERIFY2(serial_port->isMock(), "The serial port should be virtual now.");

    // benchmark the command
    qint64 mock_elapsed_time = 0;
    {  // real port
        qint64 local_elapsed_time;
        if (!measureCommandWithResponse(serial_port.get(), query_status, &local_elapsed_time)) {
            QFAIL("There is no response from the card.");
        }
        mock_elapsed_time += local_elapsed_time;

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 but should have 7"}.arg(n)));
        }
        QVERIFY2(compareResponse(buffer, response1),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(response1, 7))));
    }

    // change back to real port, the parameters should preserve and transfer to the newly oppened port
    serial_port->setPortName(real_card_port_name_);
    QVERIFY2(serial_port->isOpen(), "The serial port should be still oppened now.");
    QVERIFY2(serial_port->isMock(), "The serial port should be still real now.");
    if (!serial_port->open(QIODevice::ReadWrite)) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }
    QVERIFY2(serial_port->isOpen(), "The serial port should be oppened by now.");
    QVERIFY2(serial_port->isReal(), "The serial port should be virtual now.");
    // benchmark the command
    {  // real port
        qint64 local_elapsed_time;
        if (!measureCommandWithResponse(serial_port.get(), query_status, &local_elapsed_time)) {
            QFAIL("There is no response from the card.");
        }
        real_elapsed_time += local_elapsed_time;

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 but should have 7"}.arg(n)));
        }
        QVERIFY2(compareResponse(buffer, response2),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(response2, 7))));
    }
    qDebug() << QString("Real card command took: %1 ms").arg(real_elapsed_time / 2);
    qDebug() << QString("Mock card command took: %1 ms").arg(mock_elapsed_time);

    QTest::setBenchmarkResult((real_elapsed_time + mock_elapsed_time) / 2.0, QTest::WalltimeMilliseconds);
}


void UnifiedSerialPortTest::realBenchmark_data()
{
    if (!real_card_present_) {
        QSKIP("Benchmark of real serial port needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    QTest::addColumn<const unsigned char*>("prepare");
    QTest::addColumn<int>("n_prepare");
    QTest::addColumn<const unsigned char*>("message");
    QTest::addColumn<const unsigned char*>("response");

    // switch relay on
    //                                             STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare1 /*  */ = nullptr;
    static const unsigned char on1[] /*      */ = {0x04, 0x11, 0x01, 0x00, 0x00, 0xea, 0x0f};
    static const unsigned char response1[] /**/ = {0x04, 0x51, 0x00, 0x01, 0x00, 0xaa, 0x0f};
    QTest::newRow("Switch on") << prepare1 << 0 << &on1[0] << &response1[0];

    // switch relay all on
    //                                             STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare2 /*  */ = nullptr;
    static const unsigned char on2[] /*      */ = {0x04, 0x11, 0xff, 0x00, 0x00, 0xec, 0x0f};
    static const unsigned char response2[] /**/ = {0x04, 0x51, 0x00, 0xff, 0x00, 0xac, 0x0f};
    QTest::newRow("Switch all on") << prepare2 << 0 << &on2[0] << &response2[0];

    // switch relay off
    static const unsigned char prepare3[] = {
        //   STX   CMD   MASK  PAR1  PAR2  CHK   ETX
        /**/ 0x04, 0x11, 0x02, 0x00, 0x00, 0xe9, 0x0f  // switch relay on
    };
    //                                             STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char off3[] /*     */ = {0x04, 0x12, 0x02, 0x00, 0x00, 0xe8, 0x0f};
    static const unsigned char response3[] /**/ = {0x04, 0x51, 0x02, 0x00, 0x00, 0xa9, 0x0f};
    QTest::newRow("Switch off") << &prepare3[0] << 1 << &off3[0] << &response3[0];

    // toggle relay on
    //                                             STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare4 /*  */ = nullptr;
    static const unsigned char toggle4[] /*  */ = {0x04, 0x14, 0x04, 0x00, 0x00, 0xe4, 0x0f};
    static const unsigned char response4[] /**/ = {0x04, 0x51, 0x00, 0x04, 0x00, 0xa7, 0x0f};
    QTest::newRow("toggle on") << prepare4 << 0 << &toggle4[0] << &response4[0];

    // toggle relay off
    static const unsigned char prepare5[] = {
        //   STX   CMD   MASK  PAR1  PAR2  CHK   ETX
        /**/ 0x04, 0x11, 0x08, 0x00, 0x00, 0xe3, 0x0f  // switch relay on
    };
    //                                             STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char toggle5[] /*  */ = {0x04, 0x14, 0x08, 0x00, 0x00, 0xe0, 0x0f};
    static const unsigned char response5[] /**/ = {0x04, 0x51, 0x08, 0x00, 0x00, 0xa3, 0x0f};
    QTest::newRow("toggle off") << &prepare5[0] << 1 << &toggle5[0] << &response5[0];

    // set button mode
    static const unsigned char prepare6[] = {
        //   STX   CMD   MASK  PAR1  PAR2  CHK   ETX
        /**/ 0x04, 0x21, 0x10, 0xcf, 0x20, 0xdc, 0x0f  // set relay 5 to momentary and 6 to timed and all the rest to
                                                       // toggle
    };
    //                                               STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char query_mode6[] /**/ = {0x04, 0x22, 0x00, 0x00, 0x00, 0xda, 0x0f};
    static const unsigned char response6[] /*  */ = {0x04, 0x22, 0x10, 0xcf, 0x20, 0xdb, 0x0f};
    QTest::newRow("button mode") << &prepare6[0] << 1 << &query_mode6[0] << &response6[0];

    // set duplicated button mode - precedence momentary, toggle, timed
    static const unsigned char prepare7[] = {
        //   STX   CMD   MASK  PAR1  PAR2  CHK   ETX
        /**/ 0x04, 0x21, 0x18, 0xdf, 0x70, 0x74, 0x0f  // set relay 4 and 5 to momentary and 5, 6 and 7 to timed and
                                                       // all except for 6 to toggle =>
                                                       //     momentary: 4, 5
                                                       //     toggle: 1, 2, 3, 7, 8
                                                       //     timed: 6
    };
    //                                               STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char query_mode7[] /**/ = {0x04, 0x22, 0x00, 0x00, 0x00, 0xda, 0x0f};
    static const unsigned char response7[] /*  */ = {0x04, 0x22, 0x18, 0xc7, 0x20, 0xdb, 0x0f};
    QTest::newRow("duplicated button mode") << &prepare7[0] << 1 << &query_mode7[0] << &response7[0];

    // !!! Button mode has undocumented feature to disable selected relay buttons by setting them to no mode !!!
    // Bellow, we set only one relay to mode and the rest modes are zeros, so only one button can be used, all the
    // other physicals buttons does not work.
    // test reaction, when only one relay is set and the rest is zero
    static const unsigned char prepare8[] = {
        //   STX   CMD   MASK  PAR1  PAR2  CHK   ETX
        /**/ 0x04, 0x21, 0x01, 0x00, 0x00, 0xda, 0x0f  // set relay 1 to momentary and 6 to timed and all the rest to
                                                       // toggle
    };
    //                                               STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char query_mode8[] /**/ = {0x04, 0x22, 0x00, 0x00, 0x00, 0xda, 0x0f};
    static const unsigned char response8[] /*  */ = {0x04, 0x22, 0x01, 0x00, 0x00, 0xd9, 0x0f};
    QTest::newRow("button mode - disable buttons") << &prepare8[0] << 1 << &query_mode8[0] << &response8[0];

    // set relay timer delay
    static const unsigned char prepare9[] = {
        //   STX   CMD   MASK  PAR1  PAR2  CHK   ETX
        /**/ 0x04, 0x42, 0x40, 0x00, 0x01, 0x79, 0x0f  // set relay 7 timer to one second
    };
    //                                                STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char query_timer9[] /**/ = {0x04, 0x44, 0x40, 0x00, 0x00, 0x78, 0x0f};
    // !!!BEWARE: there is mistake in the manual, if first byte of par1 is 0, a total timer delay is queried, if 1, a
    // remaining timer delay is queried
    static const unsigned char response9[] /*   */ = {0x04, 0x44, 0x40, 0x00, 0x01, 0x77, 0x0f};
    QTest::newRow("query timer delay") << &prepare9[0] << 1 << &query_timer9[0] << &response9[0];

    // test right values of factory defaults //
    //***************************************//

    // query relay status
    //                                                  STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare10 /*      */ = nullptr;
    static const unsigned char query_status10[] /**/ = {0x04, 0x18, 0x00, 0x00, 0x00, 0xe4, 0x0f};
    static const unsigned char response10[] /*    */ = {0x04, 0x51, 0x00, 0x00, 0x00, 0xab, 0x0f};
    QTest::newRow("factory defaults - button status") << prepare10 << 0 << &query_status10[0] << &response10[0];

    // query button modes
    //                                                STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare11 /*    */ = nullptr;
    static const unsigned char query_mode11[] /**/ = {0x04, 0x22, 0x00, 0x00, 0x00, 0xda, 0x0f};
    static const unsigned char response11[] /*  */ = {0x04, 0x22, 0x00, 0xff, 0x00, 0xdb, 0x0f};
    QTest::newRow("factory defaults - button modes") << prepare11 << 0 << &query_mode11[0] << &response11[0];

    // query timer delays
    // timer 1
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare12 /*     */ = nullptr;
    static const unsigned char query_timer12[] /**/ = {0x04, 0x44, 0x01, 0x00, 0x00, 0xb7, 0x0f};
    static const unsigned char response12[] /*   */ = {0x04, 0x44, 0x01, 0x00, 0x05, 0xb2, 0x0f};
    QTest::newRow("factory defaults - query timer 1") << prepare12 << 0 << &query_timer12[0] << &response12[0];

    // timer 2
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare13 /*     */ = nullptr;
    static const unsigned char query_timer13[] /**/ = {0x04, 0x44, 0x02, 0x00, 0x00, 0xb6, 0x0f};
    static const unsigned char response13[] /*   */ = {0x04, 0x44, 0x02, 0x00, 0x05, 0xb1, 0x0f};
    QTest::newRow("factory defaults - query timer 2") << prepare13 << 0 << &query_timer13[0] << &response13[0];

    // timer 3
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare14 /*     */ = nullptr;
    static const unsigned char query_timer14[] /**/ = {0x04, 0x44, 0x04, 0x00, 0x00, 0xb4, 0x0f};
    static const unsigned char response14[] /*   */ = {0x04, 0x44, 0x04, 0x00, 0x05, 0xaf, 0x0f};
    QTest::newRow("factory defaults - query timer 3") << prepare14 << 0 << &query_timer14[0] << &response14[0];

    // timer 4
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare15 /*     */ = nullptr;
    static const unsigned char query_timer15[] /**/ = {0x04, 0x44, 0x08, 0x00, 0x00, 0xb0, 0x0f};
    static const unsigned char response15[] /*   */ = {0x04, 0x44, 0x08, 0x00, 0x05, 0xab, 0x0f};
    QTest::newRow("factory defaults - query timer 4") << prepare15 << 0 << &query_timer15[0] << &response15[0];

    // timer 5
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare16 /*     */ = nullptr;
    static const unsigned char query_timer16[] /**/ = {0x04, 0x44, 0x10, 0x00, 0x00, 0xa8, 0x0f};
    static const unsigned char response16[] /*   */ = {0x04, 0x44, 0x10, 0x00, 0x05, 0xa3, 0x0f};
    QTest::newRow("factory defaults - query timer 5") << prepare16 << 0 << &query_timer16[0] << &response16[0];

    // timer 6
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare17 /*     */ = nullptr;
    static const unsigned char query_timer17[] /**/ = {0x04, 0x44, 0x20, 0x00, 0x00, 0x98, 0x0f};
    static const unsigned char response17[] /*   */ = {0x04, 0x44, 0x20, 0x00, 0x05, 0x93, 0x0f};
    QTest::newRow("factory defaults - query timer 6") << prepare17 << 0 << &query_timer17[0] << &response17[0];

    // timer 7
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare18 /*     */ = nullptr;
    static const unsigned char query_timer18[] /**/ = {0x04, 0x44, 0x40, 0x00, 0x00, 0x78, 0x0f};
    static const unsigned char response18[] /*   */ = {0x04, 0x44, 0x40, 0x00, 0x05, 0x73, 0x0f};
    QTest::newRow("factory defaults - query timer 7") << prepare18 << 0 << &query_timer18[0] << &response18[0];

    // timer 8
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char* prepare19 /*     */ = nullptr;
    static const unsigned char query_timer19[] /**/ = {0x04, 0x44, 0x80, 0x00, 0x00, 0x38, 0x0f};
    static const unsigned char response19[] /*   */ = {0x04, 0x44, 0x80, 0x00, 0x05, 0x33, 0x0f};
    QTest::newRow("factory defaults - query timer 8") << prepare19 << 0 << &query_timer19[0] << &response19[0];
}


void UnifiedSerialPortTest::realBenchmark()
{
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // fetch data
    QFETCH(const unsigned char*, prepare);
    QFETCH(int, n_prepare);
    QFETCH(const unsigned char*, message);
    QFETCH(const unsigned char*, response);

    // prepare relay for the command
    for (int i = 0; i < n_prepare; ++i) {
        sendCommand(serial_port.get(), &prepare[i * 7]);
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(kCommandTimeoutMs - kDelayBetweenCommandsMs));
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    serial_port->readAll();

    // benchmark the command
    qint64 elapsed_time;
    if (!measureCommandWithResponse(serial_port.get(), message, &elapsed_time)) {
        QFAIL("There is no response from the card.");
    }
    QTest::setBenchmarkResult(elapsed_time, QTest::WalltimeMilliseconds);

    // check for expected response
    QByteArray data = serial_port->readAll();
    int n = data.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
    if (n != 7) {
        QFAIL(qPrintable(QString{"Response has %1 but should have 7"}.arg(n)));
    }
    QVERIFY2(compareResponse(buffer, response),
        qPrintable(QString{"The response '%1' does not match the expected %2."}
                       .arg(serial_utils::byte_to_hex(buffer, 7))
                       .arg(serial_utils::byte_to_hex(response, 7))));
}


void UnifiedSerialPortTest::realJumperStatus()
{
    if (!real_card_present_) {
        QSKIP("Benchmark of real serial port needs real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                                STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char query_jumper[] /**/ = {0x04, 0x70, 0x00, 0x00, 0x00, 0x8c, 0x0f};
    static const unsigned char jumper_off[] /*  */ = {0x04, 0x70, 0x00, 0x00, 0x00, 0x8c, 0x0f};
    qint64 elapsed_time;
    if (!measureCommandWithResponse(serial_port.get(), query_jumper, &elapsed_time)) {
        QFAIL("There is no response from the card.");
    }
    QTest::setBenchmarkResult(elapsed_time, QTest::WalltimeMilliseconds);

    // check for expected response
    QByteArray data = serial_port->readAll();
    int n = data.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
    if (n != 7) {
        QFAIL(qPrintable(QString{"Response has %1 but should have 7"}.arg(n)));
    }
    QVERIFY2(buffer[1] == jumper_off[1],
        qPrintable(QString{"The response '%1' does not match the expected %2."}
                       .arg(serial_utils::byte_to_hex(&buffer[1], 1))
                       .arg(serial_utils::byte_to_hex(&jumper_off[1], 1))));
    if (buffer[3]) {
        qDebug() << "Jumper is switched on.";
    } else {
        qDebug() << "Jumper is switched off.";
    }
}


void UnifiedSerialPortTest::realFirmwareVersion()
{
    if (!real_card_present_) {
        QSKIP("This test needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char query_version[] /**/ = {0x04, 0x71, 0x00, 0x00, 0x00, 0x8b, 0x0f};
    static const unsigned char version[] /*      */ = {0x04, 0x71, 0x00, 0x00, 0x00, 0x8b, 0x0f};
    qint64 elapsed_time;
    if (!measureCommandWithResponse(serial_port.get(), query_version, &elapsed_time)) {
        QFAIL("There is no response from the card.");
    }
    QTest::setBenchmarkResult(elapsed_time, QTest::WalltimeMilliseconds);

    // check for expected response
    QByteArray data = serial_port->readAll();
    int n = data.size();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
    if (n != 7) {
        QFAIL(qPrintable(QString{"Response has %1 but should have 7 bytes"}.arg(n)));
    }
    QVERIFY2(buffer[1] == version[1],
        qPrintable(QString{"The response '%1' does not match the expected %2."}
                       .arg(serial_utils::byte_to_hex(&buffer[1], 1))
                       .arg(serial_utils::byte_to_hex(&version[1], 1))));
    qDebug() << QString{"Firmware version is: year = %1, week = %2."}
                    .arg(2000 + static_cast<int>(buffer[3]))
                    .arg(static_cast<int>(buffer[4]));
}


void UnifiedSerialPortTest::realQueryAllTimers()
{
    if (!real_card_present_) {
        QSKIP("This test needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                           STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char query_timers[] = {0x04, 0x44, 0xff, 0x00, 0x00, 0xb9, 0x0f};
    static const unsigned char responses[] = {     /* wrap
        STX   CMD   MASK  PAR1  PAR2  CHK   ETX */
        0x04, 0x44, 0x01, 0x00, 0x05, 0xb2, 0x0f,  // wrap
        0x04, 0x44, 0x02, 0x00, 0x05, 0xb1, 0x0f,  // wrap
        0x04, 0x44, 0x04, 0x00, 0x05, 0xaf, 0x0f,  // wrap
        0x04, 0x44, 0x08, 0x00, 0x05, 0xab, 0x0f,  // wrap
        0x04, 0x44, 0x10, 0x00, 0x05, 0xa3, 0x0f,  // wrap
        0x04, 0x44, 0x20, 0x00, 0x05, 0x93, 0x0f,  // wrap
        0x04, 0x44, 0x40, 0x00, 0x05, 0x73, 0x0f,  // wrap
        0x04, 0x44, 0x80, 0x00, 0x05, 0x33, 0x0f};

    // measure time to get all responses and compute number of chunks
    qint64 elapsed_ms;
    QTimer timer;
    timer.setSingleShot(true);
    QElapsedTimer elapsed_timer;

    std::list<int> remaining_responses{0, 7, 14, 21, 28, 35, 42, 49};
    std::list<int> chunk_list;

    elapsed_timer.start();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    serial_port->write(reinterpret_cast<const char*>(query_timers), 7);
    serial_port->flush();

    timer.start(kCommandTimeoutMs * 8);
    while (!remaining_responses.empty() && timer.isActive()) {
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        loop.exec();

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n % 7 != 0) {
            QFAIL(qPrintable(QString{"Response has %1 bytes which is not a multiple of 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        std::list<int>::iterator it;
        for (int i = 0; i < n; i += 7) {
            it = std::find_if(remaining_responses.begin(), remaining_responses.end(),
                [&buffer, &i, this](int j) { return compareResponse(&buffer[i], &responses[j]); });
            QVERIFY2(it != remaining_responses.end(),
                qPrintable(QString{"The response '%1' does not match."}.arg(serial_utils::byte_to_hex(&buffer[i], 7))));
            remaining_responses.erase(it);
        }
        chunk_list.emplace_back(n / 7);
    }
    elapsed_ms = elapsed_timer.elapsed();
    QVERIFY2(timer.isActive(),
        qPrintable(QString{"Only %1 out of %2 responses were obtained."}.arg(8 - remaining_responses.size()).arg(8)));
    QTest::setBenchmarkResult(elapsed_ms, QTest::WalltimeMilliseconds);
    QString chunk_str = QString{"{%1"}.arg(*chunk_list.cbegin());
    for_each(std::next(chunk_list.cbegin()), chunk_list.cend(),
        [&chunk_str](int i) { chunk_str.append(QString{", %1"}.arg(i)); });
    chunk_str.append("}");
    qDebug() << QString{"Responses were gathered in %1."}.arg(chunk_str);
}


void UnifiedSerialPortTest::realSetMoreTimers()
{
    if (!real_card_present_) {
        QSKIP("This test needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                                STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char set_timers1[] /* */ = {0x04, 0x42, 0x0f, 0x00, 0x01, 0xaa, 0x0f};
    static const unsigned char set_timers2[] /* */ = {0x04, 0x42, 0x30, 0x00, 0x02, 0x88, 0x0f};
    static const unsigned char set_timers3[] /* */ = {0x04, 0x42, 0xc0, 0x00, 0x03, 0xf7, 0x0f};
    static const unsigned char query_timers[] /**/ = {0x04, 0x44, 0xff, 0x00, 0x00, 0xb9, 0x0f};
    static const unsigned char responses[] = {     /* wrap
        STX   CMD   MASK  PAR1  PAR2  CHK   ETX */
        0x04, 0x44, 0x01, 0x00, 0x01, 0xb6, 0x0f,  // wrap
        0x04, 0x44, 0x02, 0x00, 0x01, 0xb5, 0x0f,  // wrap
        0x04, 0x44, 0x04, 0x00, 0x01, 0xb3, 0x0f,  // wrap
        0x04, 0x44, 0x08, 0x00, 0x01, 0xaf, 0x0f,  // wrap
        0x04, 0x44, 0x10, 0x00, 0x02, 0xa6, 0x0f,  // wrap
        0x04, 0x44, 0x20, 0x00, 0x02, 0x96, 0x0f,  // wrap
        0x04, 0x44, 0x40, 0x00, 0x03, 0x75, 0x0f,  // wrap
        0x04, 0x44, 0x80, 0x00, 0x03, 0x35, 0x0f};

    sendCommand(serial_port.get(), set_timers1);
    sendCommand(serial_port.get(), set_timers2);
    sendCommand(serial_port.get(), set_timers3);
    std::this_thread::sleep_for(std::chrono::milliseconds(kCommandTimeoutMs - kDelayBetweenCommandsMs));

    // measure time to get all responses and compute number of chunks
    qint64 elapsed_ms;
    QTimer timer;
    timer.setSingleShot(true);
    QElapsedTimer elapsed_timer;

    std::list<int> remaining_responses{0, 7, 14, 21, 28, 35, 42, 49};
    std::list<int> chunk_list;

    elapsed_timer.start();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    serial_port->write(reinterpret_cast<const char*>(query_timers), 7);
    serial_port->flush();

    timer.start(kCommandTimeoutMs * 8);
    while (!remaining_responses.empty() && timer.isActive()) {
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        loop.exec();

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n % 7 != 0) {
            QFAIL(qPrintable(QString{"Response has %1 bytes which is not a multiple of 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        std::list<int>::iterator it;
        for (int i = 0; i < n; i += 7) {
            it = std::find_if(remaining_responses.begin(), remaining_responses.end(),
                [&buffer, &i, this](int j) { return compareResponse(&buffer[i], &responses[j]); });
            QVERIFY2(it != remaining_responses.end(),
                qPrintable(QString{"The response '%1' does not match."}.arg(serial_utils::byte_to_hex(&buffer[i], 7))));
            remaining_responses.erase(it);
        }
        chunk_list.emplace_back(n / 7);
    }
    elapsed_ms = elapsed_timer.elapsed();
    QVERIFY2(timer.isActive(),
        qPrintable(QString{"Only %1 out of %2 responses were obtained."}.arg(8 - remaining_responses.size()).arg(8)));
    QTest::setBenchmarkResult(elapsed_ms, QTest::WalltimeMilliseconds);
    QString chunk_str = QString{"{%1"}.arg(*chunk_list.cbegin());
    for_each(std::next(chunk_list.cbegin()), chunk_list.cend(),
        [&chunk_str](int i) { chunk_str.append(QString{", %1"}.arg(i)); });
    chunk_str.append("}");
    qDebug() << QString{"Responses were gathered in %1."}.arg(chunk_str);
}


void UnifiedSerialPortTest::realTimer()
{
    if (!real_card_present_) {
        QSKIP("This test needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char start_timer[] /*  */ = {0x04, 0x41, 0x10, 0x00, 0x03, 0xa8, 0x0f};
    static const unsigned char on_status[] /*    */ = {0x04, 0x51, 0x00, 0x10, 0x10, 0x8b, 0x0f};
    static const unsigned char query_timer[] /*  */ = {0x04, 0x44, 0x10, 0x01, 0x00, 0xa7, 0x0f};
    static const unsigned char remaining_timer /**/ = 0x44;
    static const unsigned char query_total[] /*  */ = {0x04, 0x44, 0x10, 0x00, 0x00, 0xa8, 0x0f};
    static const unsigned char total_timer[] /*  */ = {0x04, 0x44, 0x10, 0x00, 0x05, 0xa3, 0x0f};
    static const unsigned char off_status[] /*   */ = {0x04, 0x51, 0x10, 0x00, 0x00, 0x9b, 0x0f};

    // measure timer
    qint64 elapsed_ms;
    QElapsedTimer elapsed_timer;

    elapsed_timer.start();
    {  // start timer
        qint64 start_timer_elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), start_timer, &start_timer_elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }
        QTest::setBenchmarkResult(start_timer_elapsed_ms, QTest::WalltimeMilliseconds);

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, on_status),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(on_status, 7))));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    {  // query remaining time
        qint64 dummy_elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), query_timer, &dummy_elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(buffer[1] == remaining_timer,
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 1))
                           .arg(serial_utils::byte_to_hex(&remaining_timer, 1))));
        qDebug() << QString{"Remaining timer is %1s"}.arg(256 * buffer[3] + buffer[4]);
    }
    {  // query remaining time
        qint64 dummy_elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), query_total, &dummy_elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, total_timer),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(total_timer, 7))));
        qDebug() << QString{"Total timer is %1s"}.arg(256 * buffer[3] + buffer[4]);
    }
    {  // wait for relay timer to elapse
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        loop.exec();
        QVERIFY2(timer.isActive(), "There is no response from the card.");
        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, off_status),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(off_status, 7))));
    }
    elapsed_ms = elapsed_timer.elapsed();
    qDebug() << QString{"The timer took %1ms"}.arg(elapsed_ms);
}


void UnifiedSerialPortTest::realDefaultTimer()
{
    if (!real_card_present_) {
        QSKIP("This test needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                               STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char set_timer[] /*  */ = {0x04, 0x42, 0x20, 0x00, 0x01, 0x99, 0x0f};
    static const unsigned char start_timer[] /**/ = {0x04, 0x41, 0x20, 0x00, 0x00, 0x9b, 0x0f};
    static const unsigned char on_status[] /*  */ = {0x04, 0x51, 0x00, 0x20, 0x20, 0x6b, 0x0f};
    static const unsigned char off_status[] /* */ = {0x04, 0x51, 0x20, 0x00, 0x00, 0x8b, 0x0f};

    sendCommand(serial_port.get(), set_timer);
    std::this_thread::sleep_for(std::chrono::milliseconds(kCommandTimeoutMs - kDelayBetweenCommandsMs));

    // measure timer
    qint64 elapsed_ms;
    QElapsedTimer elapsed_timer;

    elapsed_timer.start();
    {  // start timer
        qint64 start_timer_elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), start_timer, &start_timer_elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }
        QTest::setBenchmarkResult(start_timer_elapsed_ms, QTest::WalltimeMilliseconds);

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, on_status),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(on_status, 7))));
    }
    {  // wait for relay timer to elapse
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        loop.exec();
        QVERIFY2(timer.isActive(), "There is no response from the card.");
        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, off_status),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(off_status, 7))));
    }
    elapsed_ms = elapsed_timer.elapsed();
    qDebug() << QString{"The timer took %1ms"}.arg(elapsed_ms);
}


void UnifiedSerialPortTest::realMoreTimers()
{
    if (!real_card_present_) {
        QSKIP("This test needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                               STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char start_timer[] /**/ = {0x04, 0x41, 0xc0, 0x00, 0x01, 0xfa, 0x0f};
    static const unsigned char on_status[] /*  */ = {0x04, 0x51, 0x00, 0xc0, 0xc0, 0x2b, 0x0f};
    static const unsigned char off_status[] /* */ = {0x04, 0x51, 0xc0, 0x00, 0x00, 0xeb, 0x0f};

    // measure timer
    qint64 elapsed_ms;
    QElapsedTimer elapsed_timer;

    elapsed_timer.start();
    {  // start timer
        qint64 start_timer_elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), start_timer, &start_timer_elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }
        QTest::setBenchmarkResult(start_timer_elapsed_ms, QTest::WalltimeMilliseconds);

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, on_status),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(on_status, 7))));
    }
    {  // wait for relay timer to elapse
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        loop.exec();
        QVERIFY2(timer.isActive(), "There is no response from the card.");
        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, off_status),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(off_status, 7))));
    }
    elapsed_ms = elapsed_timer.elapsed();
    qDebug() << QString{"The timer took %1ms"}.arg(elapsed_ms);
}


void UnifiedSerialPortTest::realMoreDefaultTimers()
{
    if (!real_card_present_) {
        QSKIP("This test needs a real K8090 card connected.", "unified_serial_port_test.cpp");
    }
    std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
    if (!serial_port) {
        QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
    }

    // benchmark the command
    //                                                 STX   CMD   MASK  PAR1  PAR2  CHK   ETX
    static const unsigned char set_timers1[] /*  */ = {0x04, 0x42, 0x0f, 0x00, 0x01, 0xaa, 0x0f};
    static const unsigned char set_timers2[] /*  */ = {0x04, 0x42, 0x30, 0x00, 0x02, 0x88, 0x0f};
    static const unsigned char set_timers3[] /*  */ = {0x04, 0x42, 0xc0, 0x00, 0x03, 0xf7, 0x0f};
    static const unsigned char start_timer1[] /* */ = {0x04, 0x41, 0x55, 0x00, 0x00, 0x66, 0x0f};
    static const unsigned char start_timer2[] /* */ = {0x04, 0x41, 0xaa, 0x00, 0x00, 0x11, 0x0f};
    static const unsigned char on_status1[] /*   */ = {0x04, 0x51, 0x00, 0x55, 0x55, 0x01, 0x0f};
    static const unsigned char on_status2[] /*   */ = {0x04, 0x51, 0x55, 0xff, 0xff, 0x58, 0x0f};
    static const unsigned char on[] /*           */ = {0x04, 0x11, 0x01, 0x00, 0x00, 0xea, 0x0f};
    static const unsigned char off[] /*          */ = {0x04, 0x12, 0x01, 0x00, 0x00, 0xe9, 0x0f};
    static const unsigned char off_status0[] /*  */ = {0x04, 0x51, 0xff, 0xfe, 0xfe, 0xb0, 0x0f};
    static const unsigned char toggle[] /*       */ = {0x04, 0x14, 0x02, 0x00, 0x00, 0xe6, 0x0f};
    static const unsigned char toggle_status[] /**/ = {0x04, 0x51, 0xfe, 0xfc, 0xfc, 0xb5, 0x0f};
    static const unsigned char off_status1[] /*  */ = {0x04, 0x51, 0xfc, 0xf0, 0xf0, 0xcf, 0x0f};
    static const unsigned char off_status2[] /*  */ = {0x04, 0x51, 0xf0, 0xc0, 0xc0, 0x3b, 0x0f};
    static const unsigned char off_status3[] /*  */ = {0x04, 0x51, 0xc0, 0x00, 0x00, 0xeb, 0x0f};

    sendCommand(serial_port.get(), set_timers1);
    sendCommand(serial_port.get(), set_timers2);
    sendCommand(serial_port.get(), set_timers3);
    std::this_thread::sleep_for(std::chrono::milliseconds(kCommandTimeoutMs - kDelayBetweenCommandsMs));

    // measure timer
    qint64 total_elapsed_ms;
    QElapsedTimer off_elapsed_timer;
    QElapsedTimer toggle_elapsed_timer;
    QElapsedTimer elapsed_timer1;
    QElapsedTimer elapsed_timer2;
    QElapsedTimer elapsed_timer3;

    off_elapsed_timer.start();
    toggle_elapsed_timer.start();
    elapsed_timer1.start();
    elapsed_timer2.start();
    elapsed_timer3.start();
    qint64 start_timer_elapsed_ms = 0;
    {  // start timers 1
        qint64 elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), start_timer1, &elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }
        start_timer_elapsed_ms += elapsed_ms;

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, on_status1),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(on_status1, 7))));
    }
    {  // start timers 2
        qint64 elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), start_timer2, &elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }
        start_timer_elapsed_ms += elapsed_ms;

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, on_status2),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(on_status2, 7))));
    }
    QTest::setBenchmarkResult(start_timer_elapsed_ms / 2.0, QTest::WalltimeMilliseconds);
    // on command should not take any effect
    sendCommand(serial_port.get(), on);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {  // interupt one timer
        qint64 elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), off, &elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }
        elapsed_ms = off_elapsed_timer.elapsed();
        qDebug() << QString{"The timer 1 was switched off after %1ms"}.arg(elapsed_ms);

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, off_status0),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(off_status0, 7))));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    {  // interupt one timer
        qint64 elapsed_ms;
        if (!measureCommandWithResponse(serial_port.get(), toggle, &elapsed_ms)) {
            QFAIL("There is no response from the card.");
        }
        elapsed_ms = toggle_elapsed_timer.elapsed();
        qDebug() << QString{"The timer 2 was toggled off after %1ms"}.arg(elapsed_ms);

        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, toggle_status),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(toggle_status, 7))));
    }
    {  // wait for relay timer group 1 to elapse
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        loop.exec();
        QVERIFY2(timer.isActive(), "There is no response from the card.");
        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, off_status1),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(off_status1, 7))));
    }
    total_elapsed_ms = elapsed_timer1.elapsed();
    qDebug() << QString{"The timer group 1 took %1ms"}.arg(total_elapsed_ms);
    {  // wait for relay timer group 2 to elapse
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        loop.exec();
        QVERIFY2(timer.isActive(), "There is no response from the card.");
        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, off_status2),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(off_status2, 7))));
    }
    total_elapsed_ms = elapsed_timer2.elapsed();
    qDebug() << QString{"The timer group 2 took %1ms"}.arg(total_elapsed_ms);
    {  // wait for relay timer group 3 to elapse
        QTimer timer;
        timer.setSingleShot(true);
        QEventLoop loop;
        connect(serial_port.get(), &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
        connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(3000);
        loop.exec();
        QVERIFY2(timer.isActive(), "There is no response from the card.");
        // check for expected response
        QByteArray data = serial_port->readAll();
        int n = data.size();
        if (n != 7) {
            QFAIL(qPrintable(QString{"Response has %1 bytes but expected 7."}.arg(n)));
        }
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const unsigned char* buffer = reinterpret_cast<const unsigned char*>(data.constData());
        QVERIFY2(compareResponse(buffer, off_status3),
            qPrintable(QString{"The response '%1' does not match the expected %2."}
                           .arg(serial_utils::byte_to_hex(buffer, 7))
                           .arg(serial_utils::byte_to_hex(off_status3, 7))));
    }
    total_elapsed_ms = elapsed_timer3.elapsed();
    qDebug() << QString{"The timer group 3 took %1ms"}.arg(total_elapsed_ms);
}


void UnifiedSerialPortTest::cleanupTestCase()
{
    if (real_card_present_) {
        std::unique_ptr<UnifiedSerialPort> serial_port = createSerialPort(real_card_port_name_);
        if (!serial_port) {
            QFAIL(qPrintable(QString{"Port '%1' can't be opened."}.arg(real_card_port_name_)));
        }
    }
}


std::unique_ptr<UnifiedSerialPort> UnifiedSerialPortTest::createSerialPort(QString port_name) const
{
    std::unique_ptr<UnifiedSerialPort> serial_port{new UnifiedSerialPort};
    serial_port->setPortName(port_name);
    serial_port->setBaudRate(QSerialPort::Baud19200);
    serial_port->setDataBits(QSerialPort::Data8);
    serial_port->setParity(QSerialPort::NoParity);
    serial_port->setStopBits(QSerialPort::OneStop);
    serial_port->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_port->isOpen()) {
        if (!serial_port->open(QIODevice::ReadWrite)) {
            serial_port.reset(nullptr);
        }
    }
    resetRelays(serial_port.get());
    return serial_port;
}


void UnifiedSerialPortTest::resetRelays(UnifiedSerialPort* serial_port) const
{
    const unsigned char factory_defaults[] = {0x04, 0x66, 0x00, 0x00, 0x00, 0x96, 0x0f};
    sendCommand(serial_port, factory_defaults);
    std::this_thread::sleep_for(std::chrono::milliseconds(kFactoryDefaultsTimeoutMs - kDelayBetweenCommandsMs));
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    serial_port->readAll();
}


bool UnifiedSerialPortTest::compareResponse(const unsigned char* response, const unsigned char* expected)
{
    unsigned char check_sum = k8090::impl_::check_sum(expected, 5);
    if (check_sum != expected[5]) {
        qDebug() << "Check sum should be:" << serial_utils::byte_to_hex(&check_sum, 1);
        return false;
    }
    for (int i = 0; i < 7; ++i) {
        if (response[i] != expected[i]) {
            return false;
        }
    }
    return true;
}


void UnifiedSerialPortTest::sendCommand(UnifiedSerialPort* serial_port, const unsigned char* command) const
{
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    serial_port->write(reinterpret_cast<const char*>(command), 7);
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents(QEventLoop::AllEvents);
    std::this_thread::sleep_for(std::chrono::milliseconds(kDelayBetweenCommandsMs));
    QCoreApplication::sendPostedEvents();
    QCoreApplication::processEvents(QEventLoop::AllEvents);
}


bool UnifiedSerialPortTest::measureCommandWithResponse(
    UnifiedSerialPort* serial_port, const unsigned char* message, qint64* elapsed_ms)
{
    QTimer timer;
    timer.setSingleShot(true);
    QEventLoop loop;
    connect(serial_port, &UnifiedSerialPort::readyRead, &loop, &QEventLoop::quit);
    connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    QElapsedTimer elapsed_timer;
    elapsed_timer.start();
    // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
    serial_port->write(reinterpret_cast<const char*>(message), 7);
    serial_port->flush();
    timer.start(kCommandTimeoutMs);
    loop.exec();
    *elapsed_ms = elapsed_timer.elapsed();
    if (!timer.isActive()) {
        return false;
    } else {
        return true;
    }
}

}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
