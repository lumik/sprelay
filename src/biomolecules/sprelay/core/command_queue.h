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
 * \file      command_queue.h
 * \brief     The biomolecules::sprelay::core::command_queue::CommandQueue class and some other related stuff.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-03-07
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_CORE_COMMAND_QUEUE_H_
#define BIOMOLECULES_SPRELAY_CORE_COMMAND_QUEUE_H_

#include <cstddef>
#include <limits>
#include <memory>
#include <mutex>
#include <queue>

#include <QList>

namespace biomolecules {
namespace sprelay {
namespace core {
namespace command_queue {

namespace impl_ {

/// \brief Helper class used for command priorities storage and comparisons.
/// \headerfile ""
template<typename TCommand>
struct CommandPriority
{
    unsigned int stamp;
    std::unique_ptr<TCommand> command;

    bool operator<(const CommandPriority& other) const
    {
        if (command->priority != other.command->priority) {
            return command->priority < other.command->priority;
        }
        return stamp > other.stamp;
    }

    void setPriority(int p) { command->priority = p; }
};


/// \brief Helper class for pending commands storage.
/// \headerfile ""
template<typename TCommand, int tSize, template<typename> class TList = QList>
class PendingCommands
{
public:
    PendingCommands() : pending_commands_{} {}
    const TList<const TCommand*>& operator[](std::size_t id) const { return pending_commands_[id]; }
    TList<const TCommand*>& operator[](std::size_t id) { return pending_commands_[id]; }
    void updateEntry(int idx, const TCommand& command)
    {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-const-cast)
        *const_cast<TCommand*>(pending_commands_[TCommand::idAsNumber(command.id)][idx]) = command;
    }

private:
    TList<const TCommand*> pending_commands_[tSize];
};

}  // namespace impl_

/// \brief Queue used for storing command before invokations.
/// \headerfile ""
template<typename TCommand, int tSize, template<typename> class TList = QList>
class CommandQueue : private std::priority_queue<impl_::CommandPriority<TCommand>>
{
public:
    CommandQueue();

    bool empty() const { return std::priority_queue<impl_::CommandPriority<TCommand>>::empty(); }
    std::size_t size() const { return std::priority_queue<impl_::CommandPriority<TCommand>>::size(); }
    bool push(const TCommand& command, bool unique = true);
    TCommand pop();
    const TList<const TCommand*>& get(typename TCommand::IdType command_id) const;
    unsigned int stampCounter() const { return stamp_counter_; }
    bool updateCommand(int idx, const TCommand& command);

private:
    void updatePriorities(typename TCommand::NumberType command_id, int idx, int priority);

    impl_::PendingCommands<TCommand, tSize, TList> pending_commands_;
    bool unique_[tSize];
    const TCommand none_command_;
    const TList<const TCommand*> none_list_;

    unsigned int stamp_counter_;
};

}  // namespace command_queue
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

#include "command_queue.tpp"

#endif  // BIOMOLECULES_SPRELAY_CORE_COMMAND_QUEUE_H_
