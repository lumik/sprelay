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
 * \file      k8090_defines.h
 * \brief     Public defines related to K8090 class.

 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-16
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_CORE_K8090_DEFINES_H_
#define SPRELAY_CORE_K8090_DEFINES_H_

#include <QMetaType>

#include <type_traits>

#include "enum_flags/enum_flags.h"

namespace sprelay {
namespace core {
namespace k8090 {

enum class CommandID : unsigned int
{
    RELAY_ON,
    RELAY_OFF,
    TOGGLE_RELAY,
    QUERY_RELAY,
    SET_BUTTON_MODE,
    BUTTON_MODE,
    START_TIMER,
    SET_TIMER,
    TIMER,
    RESET_FACTORY_DEFAULTS,
    JUMPER_STATUS,
    FIRMWARE_VERSION,
    NONE
};


enum class ResponseID : unsigned int
{
    BUTTON_MODE,       /*!< Response with button mode. */
    TIMER,             /*!< Response with timer delay. */
    BUTTON_STATUS,
    RELAY_STATUS,
    JUMPER_STATUS,     /*!< Response with jumper status. */
    FIRMWARE_VERSION,  /*!< Response with firmware version. */
    NONE               /*!< The number of all responses represents also none response. */
};


enum struct RelayID : unsigned char
{
    NONE  = 0,  /*!< None relay */
    ONE   = 1 << 0,
    TWO   = 1 << 1,
    THREE = 1 << 2,
    FOUR  = 1 << 3,
    FIVE  = 1 << 4,
    SIX   = 1 << 5,
    SEVEN = 1 << 6,
    EIGHT = 1 << 7,
    ALL   = 0xff     /*!< All relays. */
};


// this redefinition enables bitwise operator usage
constexpr bool enable_bitmask_operators(RelayID) { return true; }


constexpr RelayID from_number(unsigned int number) { return static_cast<RelayID>(1 << (number)); }


// conversion of scoped enum to underlying_type
template<typename E>
constexpr typename std::enable_if<std::is_enum<E>::value, std::underlying_type<E>>::type::type as_number(const E e)
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}


}  // namespace k8090
}  // namespace core
}  // namespace sprelay

Q_DECLARE_METATYPE(sprelay::core::k8090::RelayID)

/*!
    \enum sprelay::core::k8090::CommandID
    \ingroup k8090
    \brief Scoped enumeration listing all commands.

    See the Velleman %K8090 card manual.
*/
/*!
    \var sprelay::core::k8090::CommandID::RELAY_ON
    \brief Switch realy on command.
*/
/*!
    \var sprelay::core::k8090::CommandID::RELAY_OFF
    \brief Switch realy off command.
*/
/*!
    \var sprelay::core::k8090::CommandID::TOGGLE_RELAY
    \brief Toggle realy command.
*/
/*!
    \var sprelay::core::k8090::CommandID::QUERY_RELAY
    \brief Query relay status command.
*/
/*!
    \var sprelay::core::k8090::CommandID::SET_BUTTON_MODE
    \brief Set button mode command.
*/
/*!
    \var sprelay::core::k8090::CommandID::BUTTON_MODE
    \brief Query button mode command.
*/
/*!
    \var sprelay::core::k8090::CommandID::START_TIMER
    \brief Start relay timer command.
*/
/*!
    \var sprelay::core::k8090::CommandID::SET_TIMER
    \brief Set relay timer delay command.
*/
/*!
    \var sprelay::core::k8090::CommandID::TIMER
    \brief Query timer delay command.
*/
/*!
    \var sprelay::core::k8090::CommandID::RESET_FACTORY_DEFAULTS
    \brief Reset factory defaults command.
*/
/*!
    \var sprelay::core::k8090::CommandID::JUMPER_STATUS
    \brief Jumper status command.
*/
/*!
    \var sprelay::core::k8090::CommandID::FIRMWARE_VERSION
    \brief Firmware version command.
*/
/*!
    \var sprelay::core::k8090::CommandID::NONE
    \brief The number of all commands represents also none command.
*/

/*!
    \enum sprelay::core::k8090::ResponseID
    \ingroup k8090
    \brief Scoped enumeration listing all responses.

    See the Velleman %K8090 card manual.
*/
/*!
    \var sprelay::core::k8090::ResponseID::BUTTON_MODE
    \brief Response with button mode.
*/
/*!
    \var sprelay::core::k8090::ResponseID::TIMER
    \brief Response with timer delay.
*/
/*!
    \var sprelay::core::k8090::ResponseID::RELAY_STATUS
    \brief Relay status event.
*/
/*!
    \var sprelay::core::k8090::ResponseID::BUTTON_STATUS
    \brief Button status event.
*/
/*!
    \var sprelay::core::k8090::ResponseID::JUMPER_STATUS
    \brief Response with jumper status.
*/
/*!
    \var sprelay::core::k8090::ResponseID::FIRMWARE_VERSION
    \brief Response with firmware version.
*/
/*!
    \var sprelay::core::k8090::ResponseID::NONE
    \brief The number of all responses represents also none response.
*/

/*!
    \enum sprelay::core::k8090::RelayID
    \ingroup k8090
    \brief Scoped enumeration listing all 8 relays.

    Bitwise operators are enabled for this enum by overloading k8090::enable_bitmask_operators(RelayID) function
    (see enum_flags.h for more details) and so the value of k8090::RelayID type can be also a combination of
    particular relays.
*/
/*!
    \var sprelay::core::k8090::RelayID::NONE
    \brief None relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::ONE
    \brief First relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::TWO
    \brief Second relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::THREE
    \brief Third relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::FOUR
    \brief Fourth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::FIVE
    \brief Fifth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::SIX
    \brief Sixth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::SEVEN
    \brief Seventh relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::EIGHT
    \brief Eigth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::ALL
    \brief All relays.
*/

/*!
    \fn constexpr bool sprelay::core::k8090::enable_bitmask_operators(RelayID)
    \ingroup k8090
    \brief Function overload which enables bitwise operators for RelayID enumeration. See enum_flags.h for more
    details.

    \return True to enable bitmask operators.
*/

/*!
    \fn constexpr RelayID sprelay::core::k8090::from_number(unsigned int number)
    \ingroup k8090
    \brief Converts number to RelayID scoped enumeration.

    \param number The number.
    \return The RelayID enumerator.
*/

/*!
    \fn constexpr std::underlying_type<E> sprelay::core::k8090::as_number(const E e)
    \ingroup k8090
    \brief Converts enumeration to its underlying type.

    \param e Enumerator to be converted.
    \return The enum value as underlying type.
*/

#endif  // SPRELAY_CORE_K8090_DEFINES_H_
