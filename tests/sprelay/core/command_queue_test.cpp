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

#include "command_queue_test.h"

#include <QtTest>

#include "command_queue.h"
#include "k8090.h"

namespace sprelay {
namespace core {

void CommandQueueTest::testCase()
{
    CommandQueue<K8090Traits::Command, K8090Traits::as_number(K8090Traits::CommandID::NONE)> command_queue;

    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).id, K8090Traits::CommandID::NONE);

    K8090Traits::Command cmd1{K8090Traits::CommandID::RELAY_ON, 1, 2, 3};
    command_queue.push(cmd1, 1);
    QCOMPARE(command_queue.size(), std::size_t{1});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON), cmd1);
    QCOMPARE(command_queue.stampCounter(), 1u);

    K8090Traits::Command cmd2{K8090Traits::CommandID::RELAY_OFF, 2, 3, 4};
    command_queue.push(cmd2, 2);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_OFF), cmd2);
    QCOMPARE(command_queue.stampCounter(), 2u);

    K8090Traits::Command cmd3{K8090Traits::CommandID::TOGGLE_RELAY, 3, 4, 5};
    command_queue.push(cmd3, 2);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_OFF), cmd2);
    QCOMPARE(command_queue.stampCounter(), 3u);

    K8090Traits::Command cmd4{K8090Traits::CommandID::RELAY_ON, 255, 2, 3};
    command_queue.push(cmd4, 2);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_OFF), cmd2);
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON), cmd4);
    QCOMPARE(command_queue.stampCounter(), 3u);
    QCOMPARE(command_queue.pop(), cmd4);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.stampCounter(), 3u);

    K8090Traits::Command cmd5{K8090Traits::CommandID::RELAY_ON, 255, 2, 3};
    command_queue.push(cmd5, 3);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_OFF), cmd2);
    QCOMPARE(command_queue.stampCounter(), 4u);
    QCOMPARE(command_queue.pop(), cmd5);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.stampCounter(), 4u);

    QCOMPARE(command_queue.pop(), cmd2);
    QCOMPARE(command_queue.size(), std::size_t{1});
    QCOMPARE(command_queue.stampCounter(), 4u);

    QCOMPARE(command_queue.pop(), cmd3);
    QCOMPARE(command_queue.size(), std::size_t{0});
    QCOMPARE(command_queue.stampCounter(), 0u);
}

}  // namespace core
}  // namespace sprelay
