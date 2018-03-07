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

#ifndef SPRELAY_CORE_COMMAND_QUEUE_H_
#define SPRELAY_CORE_COMMAND_QUEUE_H_

#include <queue>

#include <cstddef>

namespace sprelay {
namespace core {

template<typename TCommand, int tSize>
class CommandQueue
{
    struct CommandPriority
    {
        typename TCommand::NumberType id;
        int priority;
        unsigned int stamp;

        bool operator<(const CommandPriority &other) const
        {
            if (priority != other.priority) {
                return priority < other.priority;
            } else {
                return stamp > other.stamp;
            }
        }
    };

public:  // NOLINT(whitespace/indent)
    CommandQueue();

    bool empty() const { return command_queue_.empty(); }
    std::size_t size() const { return command_queue_.size(); }
    bool push(TCommand command, int priority);
    TCommand pop();
    TCommand get(typename TCommand::IdType command_id) const;
    unsigned int stampCounter() const { return stamp_counter_; }

private:  // NOLINT(whitespace/indent)
    bool pending_command_ids_[tSize];  // NOLINT(runtime/arrays)
    int pending_command_priorities_[tSize];  // NOLINT(runtime/arrays)
    TCommand pending_commands_[tSize];  // NOLINT(runtime/arrays)

    std::priority_queue<CommandPriority> command_queue_;

    unsigned int stamp_counter_;
};

}  // namespace core
}  // namespace sprelay

#include "command_queue.tpp"

#endif  // SPRELAY_CORE_COMMAND_QUEUE_H_
