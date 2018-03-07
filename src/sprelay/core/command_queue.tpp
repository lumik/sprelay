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

#ifndef SPRELAY_CORE_COMMAND_QUEUE_TPP_
#define SPRELAY_CORE_COMMAND_QUEUE_TPP_

#include "command_queue.h"

namespace sprelay {
namespace core {

/*!
    \file command_queue.tpp
    \brief Implementation of CommandQueue.
*/

/*!
    \class CommandQueue
    \brief Queue used for storing command before invokations.

    The queue sorts commands according to priority and time stamp. TCommand template parameter must containt `IdType`
    and `NumberType` typedefs, `id` member of type `IdType`, static method `static NumberType idAsNumber(IdType)` and
    `bool operator==(const TCommand &other) const`.

    Example usage:

    \code
    #include "command_queue.h"

    const int N = 10;

    struct Command
    {
        using IdType = unsigned int;
        using NumberType = unsigned int;

        Command() : id(N) {}
        Command(IdType id) : id(id) {}
        static NumberType idAsNumber(IdType id) { return id; }

        IdType id;

        bool operator==(const Command &other) const
        {
            if (id != other.id) {
                return false;
            } else {
                return true;
            }
        }

        bool operator!=(const Command &other) const { return !(*this == other); }
    };

    int main()
    {
        using namespace sprelay::core;
        CommandQueue<Command, N> command_queue;

        unsigned int cmd_id1 = 0;
        Command cmd1{cmd_id1};
        unsigned int priority1 = 1;
        command_queue.push(cmd1, priority1);
        command_queue.get(cmd_id1);
        Command cmd2 = command_queue.pop();

        return 0;
    }
    \endcode

    \tparam TCommand Command representation, which must containt `IdType` and `NumberType` typedefs, `id` member of
        type `IdType`, static method `static NumberType idAsNumber(IdType)` and
        `bool operator==(const TCommand &other) const`.
    \tparam tSize Number of command ids. Command ids `NumberType` should be continuous sequence of numbers, the highest
        number must be less than `tSize`.
    \remark reentrant
*/


/*!
    \brief Default constructor
*/
template<typename TCommand, int tSize>
CommandQueue<TCommand, tSize>::CommandQueue() : pending_command_ids_{}, stamp_counter_{0} {}


/*!
    \fn CommandQueue::empty()
    \return True if the queue is empty.
*/


/*!
    \fn CommandQueue::size()
    \return Number of stored items.
*/


/*!
    \brief Inserts command with given priority to the end of the queue.

    The inserted command also gets time stamp CommandQueue::stamp_counter(), which resolves priority ties. Older
    commands preceedes new ones.

    \param command
    \param priority
    \return True if the operation was successful, false otherwise.
*/
template<typename TCommand, int tSize>
bool CommandQueue<TCommand, tSize>::push(TCommand command, int priority)
{
    typename TCommand::NumberType id = TCommand::idAsNumber(command.id);
    // TODO(lumik): Use exceptions.
    if (id >= tSize || id < 0) {
        return false;
    }
    pending_commands_[id] = command;
    if (!pending_command_ids_[id]) {
        command_queue_.push(CommandPriority{id, priority, stamp_counter_++});
        pending_command_priorities_[id] = priority;
        pending_command_ids_[id] = true;
    } else if (pending_command_priorities_[id] != priority) {
        pending_command_priorities_[id] = priority;
        std::priority_queue<CommandPriority> temp_command_queue{std::move(command_queue_)};
        CommandPriority temp_priority;
        while (!temp_command_queue.empty()) {
            temp_priority = temp_command_queue.top();
            temp_command_queue.pop();
            if (temp_priority.id == id) {
                temp_priority.priority = priority;
                command_queue_.push(temp_priority);
            } else {
                command_queue_.push(temp_priority);
            }
        }
    }
    return true;
}


/*!
    \brief Removes oldest most importat (according to its priority) element from the queue and returns it to the user.

    If the queue is empty, the defalut constructed TCommand is returned.

    \return The oldest most importat element.
*/
template<typename TCommand, int tSize>
TCommand CommandQueue<TCommand, tSize>::pop()
{
    if (empty()) {
        return TCommand{};
    } else {
        typename TCommand::NumberType id = command_queue_.top().id;
        command_queue_.pop();
        TCommand command = pending_commands_[id];
        if (empty()) {
            stamp_counter_ = 0;
        }
        pending_command_ids_[id] = false;
        return command;
    }
}


/*!
    \brief Gets the command with specified command id.

    If the id is not valid or the command is not in the queue the default constructed TCommand is returned.

    \param commad_id Command id.
    \return Requested command or default constructed TCommand.
*/
template<typename TCommand, int tSize>
TCommand CommandQueue<TCommand, tSize>::get(typename TCommand::IdType command_id) const
{
    typename TCommand::NumberType id = TCommand::idAsNumber(command_id);
    // TODO(lumik): Use exceptions.
    if (id >= tSize || id < 0) {
        return TCommand{};
    }
    if (pending_command_ids_[id]) {
        return pending_commands_[id];
    } else {
        return TCommand{};
    }
}

/*!
    \fn CommandQueue::stampCounter
    \brief Gets current stamp counter.
    \return Current stamp counter.
*/

}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_COMMAND_QUEUE_TPP_
