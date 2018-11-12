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
 * \file      command_queue.tpp
 * \brief     The biomolecules::sprelay::core::command_queue::CommandQueue class and some other related stuff.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-03-07
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include <limits>

namespace biomolecules {
namespace sprelay {
namespace core {

/*!
 * \brief Namespace containing CommmandQueue.
 */
namespace command_queue {

/*!
 * \namespace biomolecules::sprelay::core::command_queue::impl_
 * \brief Namespace containing implementation details. Not intended for public use.
 */
using namespace impl_;

/*!
 * \class biomolecules::sprelay::core::command_queue::impl_::CommandPriority
 * \tparam TCommand See CommandQueue template class.
 * \remark reentrant
 */

/*!
 * \class biomolecules::sprelay::core::command_queue::impl_::PendingCommands
 * The class abuses constant pointer access. You can get from it `TList<const TCommand*>` which is directly
 * stored inside but you can also modify stored `TCommand` using `PendingCommands::updateEntry()` method (so the
 * `TCommand` is not realy const, only returned list should not be used to modify them because it can distort
 * integrity of the queue).
 *
 * \tparam TCommand See CommandQueue template class.
 * \tparam tSize See CommandQueue template class.
 * \tparam TList See CommandQueue template class.
 * \remark reentrant
 */

/*!
 * \var CommandPriority::stamp
 * \brief Time stamp for command priority determination.
 *
 * See CommandPriority::operator<().
 */

/*!
 * \var CommandPriority::command
 * \brief Owns command and manages its memory.
 *
 * Its member TCommand::priority is used to determine ordering. See
 * CommandPriority::operator<().
 */

/*!
 * \fn CommandPriority::operator<(const CommandPriority& other) const
 * \brief Defines CommandPriority ordering.
 *
 * Ordering is defined according to TCommand::priority and time stamp CommandPriority::stamp. Higher priority and
 * lower time stamp is greater.
 */

/*!
 * \fn CommandPriority::setPriority(int p)
 * \brief Sets contained command priority to p.
 *
 * \param p The priority.
 */


/*!
 * \fn PendingCommands::PendingCommands()
 * \brief Default constructor.
 */

/*!
 * \fn const TList<const TCommand*>& PendingCommands::operator[](std::size_t id) const
 * \brief Direct constant member access.
 *
 * Index id is not controlled for validity.
 *
 * \param id Index
 * \return List containing all commands with desired id.
 */

/*!
 * \fn TList<const TCommand*>& PendingCommands::operator[](std::size_t id)
 * \brief Direct member access.
 *
 * Index id is not controlled for validity.
 *
 * \param id Index
 * \return List containing all commands with desired id.
 */

/*!
 * \fn void PendingCommands::updateEntry(int idx, const TCommand& command)
 * \brief Enables update of desired command.
 *
 * Indices command.id nor idx are not controlled for validity.
 *
 * \param idx Index.
 * \param command Command for update.
 */


/*!
 * \class CommandQueue
 * The queue sorts commands according to priority and time stamp. The commands can be inserted in unique mode, where
 * old commands are replaced by newer ones but preserving their time stamps or non-unique mode in which more commands
 * with the same id can be inserted. See CommandQueue::push() for more details.
 *
 * \warning Beware time stamp overflow, see the CommandQueue::push() method description.
 *
 * Commands inside CommandQueue can be also updated using method CommandQueue::updateCommand(). To query stored
 * commands with desired id, you can use CommandQueue::get() method.
 *
 * Example usage:
 *
 * \code
 * #include "command_queue.h"
 *
 * const int N = 10;
 *
 * struct Command
 * {
 *     using IdType = unsigned int;
 *     using NumberType = unsigned int;
 *
 *     Command() : id(N) {}
 *     Command(IdType id) : id(id) {}
 *     static NumberType idAsNumber(IdType id) { return id; }
 *
 *     IdType id;
 *     int priority;
 * };
 *
 * int main()
 * {
 *     using namespace biomolecules::sprelay::core::command_queue;
 *     CommandQueue<Command, N> command_queue;
 *
 *     unsigned int cmd_id1 = 0;
 *     unsigned int priority1 = 1;
 *     Command cmd1{cmd_id1, priority1};
 *     command_queue.push(cmd1);
 *     const QList<const Command*>& cmd_list = command_queue.get(cmd_id1);
 *     Command cmd2 = *cmd_list[0];
 *     cmd2.priority = 2;
 *     command_queue.updateCommand(0, cmd2);
 *     Command cmd3 = command_queue.pop();
 *
 *     return 0;
 * }
 * \endcode
 *
 * See ConcurentCommandQueue for thread-safe version of CommandQueue.
 *
 * \tparam TCommand Command representation, which must containt `IdType` and `NumberType` typedefs, default
 *     constructor which initializes id to none value, `id` member of type `IdType`, `int priority` member, static
 *     method `static NumberType idAsNumber(IdType)`.
 * \tparam tSize Number of command ids. Command ids `NumberType` should be continuous sequence of numbers, the highest
 *     number must be less than `tSize`.
 * \tparam TList Type of container with one template parameter, which stores commands of the same id. Defaults to
 *     QList.
 * \remark reentrant
 */


/*!
 * \brief Default constructor
 */
template<typename TCommand, int tSize, template<typename> class TList>
CommandQueue<TCommand, tSize, TList>::CommandQueue()
    : unique_{true}, none_command_{}, none_list_{&none_command_}, stamp_counter_{0}
{}


/*!
 * \fn CommandQueue::empty()
 * \return True if the queue is empty.
 */


/*!
 * \fn CommandQueue::size()
 * \return Number of stored items.
 */


/*!
 * \brief Inserts command with given priority to the end of the queue.
 *
 * The inserted command also gets time stamp CommandQueue::stamp_counter(), which resolves priority ties. Older
 * commands preceedes new ones.
 *
 * Commands can be inserted in two modes. In unique mode, all previously inserted commands with the same id is cleared
 * and the new command with the time stamp of the oldest from the removed commands is inserted instead. If the
 * non-unique mode, the command is inserted without any control for the already inserted commands with the same id.
 *
 * \warning Time stamp is `usnigned int`, so beware overflows when there are many commands stored in the queue. The
 * queue is designed to be empty most of the time. When queue is empty, the time stamp is reset to zero but if the
 * queue is never empty, the time stamp increments to infinity and there is overflow risk.
 *
 * \param command Command to be inserted.
 * \param unique If the insertion should be unique.
 * \return True if the operation was successful, false otherwise.
 */
template<typename TCommand, int tSize, template<typename> class TList>
bool CommandQueue<TCommand, tSize, TList>::push(const TCommand& command, bool unique)
{
    typename TCommand::NumberType id = TCommand::idAsNumber(command.id);
    // TODO(lumik): Use exceptions.
    if (id >= tSize || id < 0) {
        return false;
    }
    // TODO(lumik): treat overflows of stamp_counter.
    if (!unique || pending_commands_[id].empty()) {  // no command with this id is inside
        CommandPriority<TCommand> command_priority{stamp_counter_++, std::unique_ptr<TCommand>{new TCommand{command}}};
        pending_commands_[id].append(command_priority.command.get());
        unique_[id] = unique;
        std::priority_queue<CommandPriority<TCommand>>::emplace(std::move(command_priority));
    } else if (unique && unique_[id]) {
        // unique push with different priorities
        updatePriorities(id, 0, command.priority);
        pending_commands_.updateEntry(0, command);
    } else if (unique && !unique_[id]) {  // wasn't unique but now it is
        // erase all previously inserted commands with the same id and copy all the remaining commands.
        std::priority_queue<CommandPriority<TCommand>> temp_command_queue;
        unsigned int stamp = std::numeric_limits<unsigned int>::max();
        for (CommandPriority<TCommand>& command_priority : this->c) {
            if (command_priority.command->id != command.id) {
                temp_command_queue.emplace(std::move(command_priority));
            } else if (command_priority.stamp < stamp) {
                stamp = command_priority.stamp;
            }
        }
        pending_commands_[id].clear();
        // insert new command
        CommandPriority<TCommand> command_priority{stamp, std::unique_ptr<TCommand>{new TCommand{command}}};
        pending_commands_[id].append(command_priority.command.get());
        unique_[id] = unique;
        temp_command_queue.emplace(std::move(command_priority));
        // move new temporary queue to stored one
        std::priority_queue<CommandPriority<TCommand>>::operator=(std::move(temp_command_queue));
    }
    return true;
}


/*!
 * \brief Removes oldest most importat (according to its priority) element from the queue and returns it to the user.
 *
 * If the queue is empty, the defalut constructed TCommand is returned.
 *
 * \return The oldest most importat element.
 */
template<typename TCommand, int tSize, template<typename> class TList>
TCommand CommandQueue<TCommand, tSize, TList>::pop()
{
    if (empty()) {
        return TCommand{};
    } else {
        TCommand command = *std::priority_queue<CommandPriority<TCommand>>::top().command;
        typename TCommand::NumberType id = TCommand::idAsNumber(command.id);
        pending_commands_[id].removeOne(std::priority_queue<CommandPriority<TCommand>>::top().command.get());
        std::priority_queue<CommandPriority<TCommand>>::pop();  // erases command which is holded in unique_ptr
        if (!pending_commands_[id].size()) {
            unique_[id] = true;
        }
        if (empty()) {
            stamp_counter_ = 0;
        }
        return command;
    }
}


/*!
 * \brief Gets a list of commands with specified command id.
 *
 * If the id is not valid or the command is not in the queue the list to default constructed TCommand is returned. The
 * pointed to commands shouldn't be changed because it can corrupt CommandQueue consistency. You can update command
 * with specific index by calling CommandQueue::updateCommand() method. The pointers in the list are valid only until
 * the command is popped out of the CommandQueue and the indices in list is consistent with the indices, which would
 * be returned by another CommandQueue::get() method call, only until the CommandQueue is modified with the command
 * with the same id. Validity of the returned list also ends with a destruction of CommandQueue.
 *
 * \param command_id Command id.
 * \return Requested command or default constructed TCommand.
 */
template<typename TCommand, int tSize, template<typename> class TList>
const TList<const TCommand*>& CommandQueue<TCommand, tSize, TList>::get(typename TCommand::IdType command_id) const
{
    typename TCommand::NumberType id = TCommand::idAsNumber(command_id);
    // TODO(lumik): Use exceptions.
    if (id >= tSize || id < 0) {
        return none_list_;
    }
    return pending_commands_[id];
}


/*!
 * \fn CommandQueue::stampCounter
 * \brief Gets current stamp counter.
 * \return Current stamp counter.
*/

/*!
 * \brief Enables command inside CommandQueue modification.
 *
 * Command is modified according to its valid index which can be determined from the list returned by the
 * CommandQueue::get() method.
 *
 * \param idx Index of the command. For more info see CommandQueue::get().
 * \param command Command which replaces the stored command.
 */
template<typename TCommand, int tSize, template<typename> class TList>
bool CommandQueue<TCommand, tSize, TList>::updateCommand(int idx, const TCommand& command)
{
    typename TCommand::NumberType id = TCommand::idAsNumber(command.id);
    if (id >= tSize || id < 0 || idx < 0 || idx >= pending_commands_[id].size()) {
        return false;
    }
    updatePriorities(id, idx, command.priority);
    pending_commands_.updateEntry(idx, command);
    return true;
}


template<typename TCommand, int tSize, template<typename> class TList>
void CommandQueue<TCommand, tSize, TList>::updatePriorities(
    typename TCommand::NumberType command_id, int idx, int priority)
{
    if (pending_commands_[command_id].at(idx)->priority != priority) {
        std::priority_queue<CommandPriority<TCommand>> temp_command_queue;
        CommandPriority<TCommand> temp_priority;
        for (CommandPriority<TCommand>& command_priority : this->c) {
            if (command_priority.command.get() == pending_commands_[command_id].at(idx)) {
                temp_priority = std::move(command_priority);
                temp_priority.setPriority(priority);
                temp_command_queue.emplace(std::move(temp_priority));
            } else {
                temp_command_queue.emplace(std::move(command_priority));
            }
        }
        std::priority_queue<CommandPriority<TCommand>>::operator=(std::move(temp_command_queue));
    }
}

}  // namespace command_queue
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
