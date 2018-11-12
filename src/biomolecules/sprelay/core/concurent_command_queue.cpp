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
 * \file      concurent_command_queue.cpp
 * \brief     The biomolecules::sprelay::core::k8090::impl_::ConcurentCommandQueue class which specializes
 *            biomolecules::sprelay::core::command_queue::CommandQueue for usage in
 *            biomolecules::sprelay::core::k8090::K8090 class in multithreaded applications.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-09-11
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */

#include "concurent_command_queue.h"

namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {
namespace impl_ {

/*!
 * \class ConcurentCommandQueue
 * \remark thread-safe
 */


/*!
 * For more details see command_queue::CommandQueue::empty().
 */
bool ConcurentCommandQueue::empty() const
{
    std::lock_guard<std::mutex> lock{global_mutex_};
    return Predecessor::empty();
}


/*!
 * For more details see command_queue::CommandQueue::pop().
 */
Command ConcurentCommandQueue::pop()
{
    std::lock_guard<std::mutex> lock{global_mutex_};
    return Predecessor::pop();
}


/*!
 * For more details see command_queue::CommandQueue::stampCounter().
 */
unsigned int ConcurentCommandQueue::stampCounter() const
{
    std::lock_guard<std::mutex> lock{global_mutex_};
    return Predecessor::stampCounter();
}

// TODO(lumik): Add tests for this method.
/*!
 * \brief Updates compatible command in queue or pushes the new command if update is not possible.
 * \param command_id Id of a new command.
 * \param mask Mask parameter of the command.
 * \param param1 First parameter of the command.
 * \param param2 Second parameter of the command.
 *
 * Tests, if compatible command is already in the queue and if so, the command is updated, otherwise a new command
 * is inserted. It also tests for CommandID::RelayOn and CommandID::RelayOff command oposites and removes possible
 * conflicts from the queue. CommandID::ToggleRelay commands are not subjected to such a test.
 */
void ConcurentCommandQueue::updateOrPush(CommandID command_id, RelayID mask, unsigned char param1, unsigned char param2)
{
    std::lock_guard<std::mutex> lock{global_mutex_};
    // TODO(lumik): don't insert query commands if set command with the same response is already inside
    // TODO(lumik): treat commands, which are directly sended better (avoid duplication)
    Command command{command_id, kPriorities[as_number(command_id)], as_number(mask), param1, param2};

    const QList<const Command*>& pending_command_list = Predecessor::get(command_id);
    // if there is no command with the same id waiting
    if (pending_command_list.isEmpty()) {
        Predecessor::push(command);
    } else if (!updateCommandImpl(command_id, command)) {
        // else try to update stored command and if it is not possible (updateCommandImpl returns false), push it to the
        // queue
        Predecessor::push(command, false);
    }

    // if the enqueued command was switch relay on or off command and there is the oposit command stored
    // TODO(lumik): test if updated oposite command doesn't update any relay and if it does, remove it from the
    // queue
    if (command_id == CommandID::RelayOn) {
        const QList<const impl_::Command*>& off_pending_command_list = Predecessor::get(CommandID::RelayOff);
        if (!off_pending_command_list.isEmpty()) {
            updateCommandImpl(CommandID::RelayOff, command);
        }
    } else if (command_id == CommandID::RelayOff) {
        const QList<const impl_::Command*>& on_pending_command_list = Predecessor::get(CommandID::RelayOn);
        if (!on_pending_command_list.isEmpty()) {
            updateCommandImpl(CommandID::RelayOn, command);
        }
    }
}


/*!
 * \brief Returns number of commands with a specified id in the queue.
 * \param command_id The command id.
 * \return The number of commands with the id.
 */
int ConcurentCommandQueue::count(CommandID command_id) const
{
    std::lock_guard<std::mutex> lock{global_mutex_};
    return Predecessor::get(command_id).size();
}


// helper method which updates already enqueued command
bool ConcurentCommandQueue::updateCommandImpl(CommandID command_id, const Command& command)
{
    const QList<const impl_::Command*>& pending_command_list = Predecessor::get(command_id);
    // check if equal command is in pending command list
    int compatible_idx = pending_command_list.size();
    for (int i = 0; i < pending_command_list.size(); ++i) {
        if (pending_command_list[i]->isCompatible(command)) {
            compatible_idx = i;
            break;
        }
    }
    if (compatible_idx != pending_command_list.size()) {
        impl_::Command insert_command = *pending_command_list[compatible_idx];
        insert_command |= command;
        if (insert_command.priority < command.priority) {
            insert_command.priority = command.priority;
        }
        Predecessor::updateCommand(compatible_idx, insert_command);
        return true;
    }
    return false;
}

}  // namespace impl_
}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
