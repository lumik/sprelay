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

#include <QCoreApplication>
#include <QtTest>

#include <memory>

#include "command_queue_test.h"
#include "k8090_test.h"
#include "k8090_utils_test.h"
#include "mock_serial_port_test.h"
#include "serial_port_utils_test.h"
#include "unified_serial_port_test.h"

using namespace sprelay::core;  // NOLINT(build/namespaces)

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);
    app.setAttribute(Qt::AA_Use96Dpi, true);
    int status = 0;

    // SerialPortUtilsTest
    std::unique_ptr<serial_utils::SerialPortUtilsTest> serial_port_utils_test{new serial_utils::SerialPortUtilsTest};
    status |= QTest::qExec(serial_port_utils_test.get(), argc, argv);

    // MockSerialPortTest
    std::unique_ptr<MockSerialPortTest> mock_serial_port_test{new MockSerialPortTest};
    status |= QTest::qExec(mock_serial_port_test.get(), argc, argv);

    // UnifiedSerialPortTest
    std::unique_ptr<UnifiedSerialPortTest> unified_serial_port_test{new UnifiedSerialPortTest};
    status |= QTest::qExec(unified_serial_port_test.get(), argc, argv);

    // CommandQueueTest
    std::unique_ptr<command_queue::CommandQueueTest> command_queue_test{new command_queue::CommandQueueTest};
    status |= QTest::qExec(command_queue_test.get(), argc, argv);

    // K8090UtilsTest
    std::unique_ptr<k8090::impl_::K8090UtilsTest> k8090_utils_test{new k8090::impl_::K8090UtilsTest};
    status |= QTest::qExec(k8090_utils_test.get(), argc, argv);

    // CommandTest
    std::unique_ptr<k8090::impl_::CommandTest> command_test{new k8090::impl_::CommandTest};
    status |= QTest::qExec(command_test.get(), argc, argv);

    // CardMessageTest
    std::unique_ptr<k8090::impl_::CardMessageTest> card_message_test{new k8090::impl_::CardMessageTest};
    status |= QTest::qExec(card_message_test.get(), argc, argv);

    // K8090Test
    std::unique_ptr<K8090Test> k8090_test{new K8090Test};
    status |= QTest::qExec(k8090_test.get(), argc, argv);

    return status;
}
