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
 * \file      k8090_utils.cpp
 * \brief     Utility functions and data structures for biomolecules::sprelay::core::k8090::K8090 class implementation.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-16
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */

#include <iterator>
#include <stdexcept>

#include "k8090_commands.h"
#include "k8090_utils.h"


namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {
namespace impl_ {


/*!
 * \enum biomolecules::sprelay::core::impl_::TimerDelayType
 * See the Velleman %K8090 card manual for more info about timer delay types.
 */


/*!
 * \struct biomolecules::sprelay::core::k8090::impl_::Command
 * It is used for command comparisons and in command_queue::CommandQueue.
 */

/*!
 * \typedef Command::IdType
 * \brief Typename of id
 *
 * Required by command_queue::CommandQueue API.
 */

/*!
 * \typedef Command::NumberType
 * \brief Underlying typename of id
 *
 * Required by command_queue::CommandQueue API.
 */

/*!
 * \fn Command::Command()
 * \brief Default constructor.
 *
 * Initializes its id member to k8090::CommandID::None so the Command can be used as return value indicating failure.
 */

/*!
 * \fn explicit Command::Command(IdType id, int priority = 0, unsigned char mask = 0, unsigned char param1 = 0,
 * unsigned char param2 = 0)
 * \brief Initializes Command.
 */

/*!
 * \fn static NumberType Command::idAsNumber(IdType id)
 * \brief Converts id to its underlying type.
 *
 * Required by command_queue::CommandQueue API.
 *
 * \param id Command id.
 * \return Underlying type representation of the id.
 */

/*!
 * \var Command::id
 * \brief Command id.
 *
 * Required by command_queue::CommandQueue API.
 */

/*!
 * \var Command::priority
 * \brief Command priority.
 *
 * Required by command_queue::CommandQueue API.
 */

/*!
 * \var Command::params
 * \brief Stores command parameters.
 */


/*!
 * \brief Merges the other Command.
 *
 * If the other command is k8090::CommandID::RelayON and is merged to k8090::CommandID::RelayOff or the
 * oposite, the negation is merged. XOR is applied to k8090::CommandID::ToggleRelay and for
 * k8090::CommandID::SetButtonMode and duplicate assignments, the command is merged according to precedence
 * stated in Velleman %K8090 card manual (momentary mode, toggle mode, timed mode from most important to less). The
 * parameters of k8090::CommandID::StartTimer, k8090::CommandID::SetTimer and k8090::CommandID::Timer are not
 * merged, only the affected relays are updated. Check, if that makes sense, is up to class user.
 *
 * The other commands are merged naturally as _or assignment_ operator to their members.
 *
 * The compatibility of commands is not checked inside the method. This method enables you to merge commands with
 * different ids. The compatibility can be checked for example by the Command::isCompatible() method.
 *
 * \param other The other command.
 * \return Merged command.
 */
Command& Command::operator|=(const Command& other)
{
    // TODO(lumik): enable merging into none command
    switch (id) {
        // commands with special treatment
        case k8090::CommandID::RelayOn:
            if (other.id == k8090::CommandID::RelayOff) {
                params[0] &= ~other.params[0];
            } else {
                params[0] |= other.params[0];
            }
            break;
        case k8090::CommandID::RelayOff:
            if (other.id == k8090::CommandID::RelayOn) {
                params[0] &= ~other.params[0];
            } else {
                params[0] |= other.params[0];
            }
            break;
        case k8090::CommandID::ToggleRelay:
            params[0] ^= other.params[0];
            break;
        case k8090::CommandID::SetButtonMode:
            params[0] |= other.params[0];
            params[1] = (params[1] | other.params[1]) & ~params[0];
            params[2] = (params[2] | other.params[2]) & ~params[1] & ~params[0];
            break;
        // commands with one relevant parameter mask
        case k8090::CommandID::StartTimer:
        case k8090::CommandID::SetTimer:
        case k8090::CommandID::Timer:
            params[0] |= other.params[0];
            break;
        // commands with no parameters
        //     case k8090::CommandID::QueryRelay :
        //     case k8090::CommandID::ButtonMode :
        //     case k8090::CommandID::ResetFactoryDefaults :
        //     case k8090::CommandID::JumperStatus :
        //     case k8090::CommandID::FirmwareVersion :
        //     case k8090::CommandID::None
        default:
            break;
    }
    return *this;
}

/*!
 * \fn bool Command::operator==(const Command& other) const
 * \brief Compares two commands for equality.
 *
 * \param other The command to be compared.
 * \return True if the commands are the same.
 */

/*!
 * \fn bool Command::operator!=(const Command& other) const
 * \brief Compares two commands for non-equality.
 *
 * \param other The command to be compared.
 * \return True if the commands are different.
 */


/*!
 * \brief Tests, if commands are compatible.
 *
 * Compatible commands can be merget by the Command::operator|=() operator.
 *
 * \param other The command to be stested.
 * \return True if the commands are compatible.
 */
bool Command::isCompatible(const Command& other) const
{
    // ids are not equal
    if (id != other.id) {
        // TODO(lumik): treat compatibility of toggle relay
        // TODO(lumik): treat compatibility to none command
        switch (id) {
            case k8090::CommandID::RelayOn:
                if (other.id == k8090::CommandID::RelayOff) {
                    return true;
                }
                return false;
            case k8090::CommandID::RelayOff:
                if (other.id == k8090::CommandID::RelayOn) {
                    return true;
                }
                return false;
            default:
                return false;
        }
    }

    // ids are equal
    switch (id) {
        // TODO(lumik): handle the cases with the same id as compatible.
        case k8090::CommandID::StartTimer:
        case k8090::CommandID::SetTimer:
            for (int i = 1; i < 3; ++i) {
                if (params[i] != other.params[i]) {
                    return false;
                }
            }
            return true;
        case k8090::CommandID::Timer:
            // compare only first bits
            if ((params[1] & 1) != (other.params[1] & 1)) {
                return false;
            }
            return true;
        default:
            return true;
    }
}


/*!
 * \param msg C array of bytes.
 * \param n The number of the bytes.
 * \return The checksum.
 *
 * The checksum is bit inversion of sum of all bytes plus 1.
 */
unsigned char check_sum(const unsigned char* msg, int n)
{
    unsigned int sum = 0u;
    for (int ii = 0; ii < n; ++ii) {
        sum += (unsigned int)msg[ii];
    }
    unsigned char sum_byte = sum % 256;
    sum = (unsigned int)(~sum_byte) + 1u;
    sum_byte = (unsigned char)sum % 256;

    return sum_byte;
}


/*!
 * \brief Constructor directly from data.
 * \param stx STX byte.
 * \param cmd Command byte.
 * \param mask Mask byte.
 * \param param1 1st parameter byte.
 * \param param2 2nd parameter byte.
 * \param chk Checksum byte.
 * \param etx ETX byte.
 */
CardMessage::CardMessage(unsigned char stx, unsigned char cmd, unsigned char mask, unsigned char param1,
    unsigned char param2, unsigned char chk, unsigned char etx)
    : data{stx, cmd, mask, param1, param2, chk, etx}
{}


/*!
 * \brief The constructor from the response acquired from QSerialPort.
 * \param begin The begin iterator.
 * \param end The end iterator.
 * \throws std::out_of_range exception if the length of response between begin and end is not 7 bytes.
 */
CardMessage::CardMessage(QByteArray::const_iterator begin, QByteArray::const_iterator end)
{
    if (std::distance(begin, end) != 7) {
        // TODO(lumik): all exceptions should derive from the project specific one. Refactor exceptions.
        throw std::out_of_range{"The card response should have exactly 7 bytes."};
    }
    for (int i = 0; i < 7; ++i) {
        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        data[i] = reinterpret_cast<const unsigned char&>(begin[i]);
    }
}


/*!
 * \brief The constructor unsigned char message.
 * \param begin The pointer to the beginning of the message.
 * \param end The pointer to one after the end of the message.
 * \throws std::out_of_range exception if the length of response between begin and end is not 7 bytes.
 */
CardMessage::CardMessage(const unsigned char* begin, const unsigned char* end)
{
    if (std::distance(begin, end) != 7) {
        // TODO(lumik): all exceptions should derive from the project specific one. Refactor exceptions.
        throw std::out_of_range{"The card response should have exactly 7 bytes."};
    }
    for (int i = 0; i < 7; ++i) {
        data[i] = begin[i];
    }
}


/*!
 * \brief Sets the checksum byte to the message checksum
 * \sa check_sum
 */
void CardMessage::checksumMessage()
{
    data[5] = check_sum(data, 5);
}


/*!
 * \brief Validates the response from card for formal requirements.
 * \return True if the response is valid.
 */
bool CardMessage::isValid() const
{
    if (data[0] != kStxByte) return false;
    unsigned char chk = check_sum(data, 5);
    if (chk != data[5]) return false;
    if (data[6] != kEtxByte) return false;
    return true;
}

/*!
 * \brief Returns the byte identifying the response type
 * \return The id byte.
 */
unsigned char CardMessage::commandByte() const
{
    return data[1];
}


/*!
 * \var CardMessage::data
 * \brief The message data.
 */

}  // namespace impl_
}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
