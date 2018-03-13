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
namespace command_queue {

void CommandQueueTest::uniquePush()
{
    CommandQueue<K8090Traits::Command, K8090Traits::as_number(K8090Traits::CommandID::NONE)> command_queue;

    // QLists with commands initialized
    QVERIFY(command_queue.get(K8090Traits::CommandID::RELAY_ON).empty());

    // push command
    const int priority1 = 1;
    K8090Traits::Command cmd1{K8090Traits::CommandID::RELAY_ON, priority1, 1, 2, 3};
    command_queue.push(cmd1);
    QCOMPARE(command_queue.size(), std::size_t{1});
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_ON)[0], cmd1);
    QCOMPARE(command_queue.stampCounter(), 1u);

    // push another command wiyh higher priority, it should preceede previously added one
    const int priority2 = 2;
    K8090Traits::Command cmd2{K8090Traits::CommandID::RELAY_OFF, priority2, 2, 3, 4};
    command_queue.push(cmd2);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_OFF)[0], cmd2);
    QCOMPARE(command_queue.stampCounter(), 2u);

    // push next command with the same priority
    K8090Traits::Command cmd3{K8090Traits::CommandID::TOGGLE_RELAY, priority2, 3, 4, 5};
    command_queue.push(cmd3);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_OFF)[0], cmd2);
    QCOMPARE(command_queue.stampCounter(), 3u);

    // override the first command with higher priority
    K8090Traits::Command cmd4{K8090Traits::CommandID::RELAY_ON, priority2, 255, 2, 3};
    command_queue.push(cmd4);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_OFF)[0], cmd2);
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_ON)[0], cmd4);
    QCOMPARE(command_queue.stampCounter(), 3u);

    // remove the top command (it should be the first one now)
    QCOMPARE(command_queue.pop(), cmd4);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.stampCounter(), 3u);

    // push command with the highest priority
    const int priority3 = 3;
    K8090Traits::Command cmd5{K8090Traits::CommandID::RELAY_ON, priority3, 255, 2, 3};
    command_queue.push(cmd5);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_OFF)[0], cmd2);
    QCOMPARE(command_queue.stampCounter(), 4u);

    // remove the top command (which should be the last one)
    QCOMPARE(command_queue.pop(), cmd5);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.stampCounter(), 4u);

    // remove the remaining commands (they should be in order of their addition)
    QCOMPARE(command_queue.pop(), cmd2);
    QCOMPARE(command_queue.size(), std::size_t{1});
    QCOMPARE(command_queue.stampCounter(), 4u);

    QCOMPARE(command_queue.pop(), cmd3);
    QCOMPARE(command_queue.size(), std::size_t{0});
    QCOMPARE(command_queue.stampCounter(), 0u);
}

void CommandQueueTest::notUniquePush()
{
    CommandQueue<K8090Traits::Command, K8090Traits::as_number(K8090Traits::CommandID::NONE)> command_queue;

    QVERIFY(command_queue.get(K8090Traits::CommandID::RELAY_ON).empty());

    // two not unique pushes
    const int priority1 = 1;
    K8090Traits::Command cmd1{K8090Traits::CommandID::RELAY_ON, priority1, 1, 2, 3};
    K8090Traits::Command cmd2{K8090Traits::CommandID::RELAY_ON, priority1, 2, 3, 4};
    command_queue.push(cmd1, false);
    command_queue.push(cmd2, false);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).size(), 2);
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_ON)[0], cmd1);
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_ON)[1], cmd2);
    QCOMPARE(command_queue.stampCounter(), 2u);

    // first unique and second not
    K8090Traits::Command cmd3{K8090Traits::CommandID::RELAY_OFF, priority1, 1, 2, 3};
    K8090Traits::Command cmd4{K8090Traits::CommandID::RELAY_OFF, priority1, 2, 3, 4};
    command_queue.push(cmd3, true);
    command_queue.push(cmd4, false);
    QCOMPARE(command_queue.size(), std::size_t{4});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_OFF).size(), 2);
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_OFF)[0], cmd3);
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_OFF)[1], cmd4);
    QCOMPARE(command_queue.stampCounter(), 4u);

    // overwrite not unique by one unique
    K8090Traits::Command cmd5{K8090Traits::CommandID::RELAY_ON, priority1, 3, 4, 5};
    command_queue.push(cmd5, true);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).size(), 1);
    QCOMPARE(*command_queue.get(K8090Traits::CommandID::RELAY_ON)[0], cmd5);
    QCOMPARE(command_queue.stampCounter(), 4u);
    // test if the time stamp was preserved
    QCOMPARE(command_queue.pop(), cmd5);
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).size(), 0);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.stampCounter(), 4u);

    // test if pop erases the right one
}

void CommandQueueTest::updateCommand()
{
    CommandQueue<K8090Traits::Command, K8090Traits::as_number(K8090Traits::CommandID::NONE)> command_queue;

    QVERIFY(command_queue.get(K8090Traits::CommandID::RELAY_ON).empty());

    // two not unique pushes
    const int priority1 = 1;
    K8090Traits::Command cmd1{K8090Traits::CommandID::RELAY_ON, priority1, 1, 2, 3};
    command_queue.push(cmd1, false);
    K8090Traits::Command cmd2{K8090Traits::CommandID::RELAY_ON, priority1, 2, 3, 4};
    command_queue.push(cmd2, false);
    const int priority2 = 2;
    K8090Traits::Command cmd3{K8090Traits::CommandID::RELAY_ON, priority2, 3, 4, 5};
    command_queue.push(cmd3, false);
    K8090Traits::Command cmd4{K8090Traits::CommandID::RELAY_ON, priority1, 4, 5, 6};
    command_queue.push(cmd4, false);
    QCOMPARE(command_queue.size(), std::size_t{4});
    const QList<const K8090Traits::Command *> & command_list1 = command_queue.get(K8090Traits::CommandID::RELAY_ON);
    QCOMPARE(command_list1.size(), 4);
    QCOMPARE(*command_list1[0], cmd1);
    QCOMPARE(*command_list1[1], cmd2);
    QCOMPARE(*command_list1[2], cmd3);
    QCOMPARE(*command_list1[3], cmd4);
    QCOMPARE(command_queue.stampCounter(), 4u);

    // update element
    const int priority3 = 3;
    cmd2 = K8090Traits::Command{K8090Traits::CommandID::RELAY_ON, priority3, 5, 6, 7};
    command_queue.updateCommand(1, cmd2);

    // pop elements
    QCOMPARE(command_queue.pop(), cmd2);
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).size(), 3);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(command_queue.pop(), cmd3);
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).size(), 2);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.pop(), cmd1);
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).size(), 1);
    QCOMPARE(command_queue.size(), std::size_t{1});
    QCOMPARE(command_queue.pop(), cmd4);
    QCOMPARE(command_queue.get(K8090Traits::CommandID::RELAY_ON).size(), 0);
    QCOMPARE(command_queue.size(), std::size_t{0});
}

}  // namespace command_queue
}  // namespace core
}  // namespace sprelay
