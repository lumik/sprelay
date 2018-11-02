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
 * \file      unified_serial_port_test.h
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


#ifndef BIOMOLECULES_SPRELAY_CORE_UNIFIED_SERIAL_PORT_TEST_H_
#define BIOMOLECULES_SPRELAY_CORE_UNIFIED_SERIAL_PORT_TEST_H_

#include <QObject>
#include <QString>

#include <memory>

#include "lumik/qtest_suite/qtest_suite.h"

namespace biomolecules {
namespace sprelay {
namespace core {

class UnifiedSerialPort;

class UnifiedSerialPortTest : public QObject
{
    Q_OBJECT
public:  // NOLINT(whitespace/indent)
    static const int kCommandTimeoutMs;
    static const int kFactoryDefaultsTimeoutMs;
    static const int kDelayBetweenCommandsMs;

private slots:  // NOLINT(whitespace/indent)
    // TODO(lumik): store relay states before tests (timers, button modes, relay statuses) and reset them at the end
    void initTestCase();
    void availablePorts();
    void switchRealVirtual();
    void realBenchmark_data();
    void realBenchmark();
    void realJumperStatus();
    void realFirmwareVersion();
    void realQueryAllTimers();
    void realSetMoreTimers();
    void realTimer();
    void realDefaultTimer();
    void realMoreTimers();
    void realMoreDefaultTimers();
    void cleanupTestCase();

private:  // NOLINT(whitespace/indent)
    std::unique_ptr<UnifiedSerialPort> createSerialPort(QString port_name) const;
    void resetRelays(UnifiedSerialPort *serial_port) const;
    bool compareResponse(const unsigned char *response, const unsigned char *expected);
    void sendCommand(UnifiedSerialPort *serial_port, const unsigned char *command) const;
    bool measureCommandWithResponse(UnifiedSerialPort *serial_port, const unsigned char *message, qint64 *elapsed_ms);
    bool real_card_present_;
    QString real_card_port_name_;
};

ADD_TEST(UnifiedSerialPortTest)

}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPRELAY_CORE_UNIFIED_SERIAL_PORT_TEST_H_
