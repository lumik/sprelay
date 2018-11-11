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
 * \file      command_queue_test.cpp
 * \brief     The biomolecules::sprelay::core::command_queue::CommandQueueTest class which implements tests for
 *            biomolecules::sprelay::core::command_queue::CommandQueue.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-03-07
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "command_queue_test.h"

#include <QtTest>

#include "biomolecules/sprelay/core/command_queue.h"
#include "biomolecules/sprelay/core/k8090.h"
#include "biomolecules/sprelay/core/k8090_utils.h"

namespace biomolecules {
namespace sprelay {
namespace core {
namespace command_queue {

using Command = biomolecules::sprelay::core::k8090::impl_::Command;

void CommandQueueTest::uniquePush()
{
    CommandQueue<Command, k8090::as_number(k8090::CommandID::None)> command_queue;

    // QLists with commands initialized
    QVERIFY(command_queue.get(k8090::CommandID::RelayOn).empty());

    // push command
    const int priority1 = 1;
    Command cmd1{k8090::CommandID::RelayOn, priority1, 1, 2, 3};
    command_queue.push(cmd1);
    QCOMPARE(command_queue.size(), std::size_t{1});
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOn)[0], cmd1);
    QCOMPARE(command_queue.stampCounter(), 1u);

    // push another command wiyh higher priority, it should preceede previously added one
    const int priority2 = 2;
    Command cmd2{k8090::CommandID::RelayOff, priority2, 2, 3, 4};
    command_queue.push(cmd2);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOff)[0], cmd2);
    QCOMPARE(command_queue.stampCounter(), 2u);

    // push next command with the same priority
    Command cmd3{k8090::CommandID::ToggleRelay, priority2, 3, 4, 5};
    command_queue.push(cmd3);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOff)[0], cmd2);
    QCOMPARE(command_queue.stampCounter(), 3u);

    // override the first command with higher priority
    Command cmd4{k8090::CommandID::RelayOn, priority2, 255, 2, 3};
    command_queue.push(cmd4);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOff)[0], cmd2);
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOn)[0], cmd4);
    QCOMPARE(command_queue.stampCounter(), 3u);

    // remove the top command (it should be the first one now)
    QCOMPARE(command_queue.pop(), cmd4);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.stampCounter(), 3u);

    // push command with the highest priority
    const int priority3 = 3;
    Command cmd5{k8090::CommandID::RelayOn, priority3, 255, 2, 3};
    command_queue.push(cmd5);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOff)[0], cmd2);
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
    CommandQueue<Command, k8090::as_number(k8090::CommandID::None)> command_queue;

    QVERIFY(command_queue.get(k8090::CommandID::RelayOn).empty());

    // two not unique pushes
    const int priority1 = 1;
    Command cmd1{k8090::CommandID::RelayOn, priority1, 1, 2, 3};
    Command cmd2{k8090::CommandID::RelayOn, priority1, 2, 3, 4};
    command_queue.push(cmd1, false);
    command_queue.push(cmd2, false);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOn).size(), 2);
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOn)[0], cmd1);
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOn)[1], cmd2);
    QCOMPARE(command_queue.stampCounter(), 2u);

    // first unique and second not
    Command cmd3{k8090::CommandID::RelayOff, priority1, 1, 2, 3};
    Command cmd4{k8090::CommandID::RelayOff, priority1, 2, 3, 4};
    command_queue.push(cmd3, true);
    command_queue.push(cmd4, false);
    QCOMPARE(command_queue.size(), std::size_t{4});
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOff).size(), 2);
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOff)[0], cmd3);
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOff)[1], cmd4);
    QCOMPARE(command_queue.stampCounter(), 4u);

    // overwrite not unique by one unique
    Command cmd5{k8090::CommandID::RelayOn, priority1, 3, 4, 5};
    command_queue.push(cmd5, true);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOn).size(), 1);
    QCOMPARE(*command_queue.get(k8090::CommandID::RelayOn)[0], cmd5);
    QCOMPARE(command_queue.stampCounter(), 4u);
    // test if the time stamp was preserved
    QCOMPARE(command_queue.pop(), cmd5);
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOn).size(), 0);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.stampCounter(), 4u);

    // test if pop erases the right one
}

void CommandQueueTest::updateCommand()
{
    CommandQueue<Command, k8090::as_number(k8090::CommandID::None)> command_queue;

    QVERIFY(command_queue.get(k8090::CommandID::RelayOn).empty());

    // two not unique pushes
    const int priority1 = 1;
    Command cmd1{k8090::CommandID::RelayOn, priority1, 1, 2, 3};
    command_queue.push(cmd1, false);
    Command cmd2{k8090::CommandID::RelayOn, priority1, 2, 3, 4};
    command_queue.push(cmd2, false);
    const int priority2 = 2;
    Command cmd3{k8090::CommandID::RelayOn, priority2, 3, 4, 5};
    command_queue.push(cmd3, false);
    Command cmd4{k8090::CommandID::RelayOn, priority1, 4, 5, 6};
    command_queue.push(cmd4, false);
    QCOMPARE(command_queue.size(), std::size_t{4});
    const QList<const Command*>& command_list1 = command_queue.get(k8090::CommandID::RelayOn);
    QCOMPARE(command_list1.size(), 4);
    QCOMPARE(*command_list1[0], cmd1);
    QCOMPARE(*command_list1[1], cmd2);
    QCOMPARE(*command_list1[2], cmd3);
    QCOMPARE(*command_list1[3], cmd4);
    QCOMPARE(command_queue.stampCounter(), 4u);

    // update element
    const int priority3 = 3;
    cmd2 = Command{k8090::CommandID::RelayOn, priority3, 5, 6, 7};
    command_queue.updateCommand(1, cmd2);

    // pop elements
    QCOMPARE(command_queue.pop(), cmd2);
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOn).size(), 3);
    QCOMPARE(command_queue.size(), std::size_t{3});
    QCOMPARE(command_queue.pop(), cmd3);
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOn).size(), 2);
    QCOMPARE(command_queue.size(), std::size_t{2});
    QCOMPARE(command_queue.pop(), cmd1);
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOn).size(), 1);
    QCOMPARE(command_queue.size(), std::size_t{1});
    QCOMPARE(command_queue.pop(), cmd4);
    QCOMPARE(command_queue.get(k8090::CommandID::RelayOn).size(), 0);
    QCOMPARE(command_queue.size(), std::size_t{0});
}

}  // namespace command_queue
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
