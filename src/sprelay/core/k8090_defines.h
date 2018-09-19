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
 * \ingroup   group_sprelay_core_public
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

enum struct CommandID : unsigned int
{
    RelayOn,
    RelayOff,
    ToggleRelay,
    QueryRelay,
    SetButtonMode,
    ButtonMode,
    StartTimer,
    SetTimer,
    Timer,
    ResetFactoryDefaults,
    JumperStatus,
    FirmwareVersion,
    None
};


enum struct ResponseID : unsigned int
{
    ButtonMode,       /*!< Response with button mode. */
    Timer,             /*!< Response with timer delay. */
    ButtonStatus,
    RelayStatus,
    JumperStatus,     /*!< Response with jumper status. */
    FirmwareVersion,  /*!< Response with firmware version. */
    None               /*!< The number of all responses represents also none response. */
};


enum struct RelayID : unsigned char
{
    None  = 0,  /*!< None relay */
    One   = 1 << 0,
    Two   = 1 << 1,
    Three = 1 << 2,
    Four  = 1 << 3,
    Five  = 1 << 4,
    Six   = 1 << 5,
    Seven = 1 << 6,
    Eight = 1 << 7,
    All   = 0xff
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
    \ingroup group_sprelay_core_public
    \brief Scoped enumeration listing all commands.

    See the Velleman %K8090 card manual.
*/
/*!
    \var sprelay::core::k8090::CommandID::RelayOn
    \brief Switch realy on command.
*/
/*!
    \var sprelay::core::k8090::CommandID::RelayOff
    \brief Switch realy off command.
*/
/*!
    \var sprelay::core::k8090::CommandID::ToggleRelay
    \brief Toggle realy command.
*/
/*!
    \var sprelay::core::k8090::CommandID::QueryRelay
    \brief Query relay status command.
*/
/*!
    \var sprelay::core::k8090::CommandID::SetButtonMode
    \brief Set button mode command.
*/
/*!
    \var sprelay::core::k8090::CommandID::ButtonMode
    \brief Query button mode command.
*/
/*!
    \var sprelay::core::k8090::CommandID::StartTimer
    \brief Start relay timer command.
*/
/*!
    \var sprelay::core::k8090::CommandID::SetTimer
    \brief Set relay timer delay command.
*/
/*!
    \var sprelay::core::k8090::CommandID::Timer
    \brief Query timer delay command.
*/
/*!
    \var sprelay::core::k8090::CommandID::ResetFactoryDefaults
    \brief Reset factory defaults command.
*/
/*!
    \var sprelay::core::k8090::CommandID::JumperStatus
    \brief Jumper status command.
*/
/*!
    \var sprelay::core::k8090::CommandID::FirmwareVersion
    \brief Firmware version command.
*/
/*!
    \var sprelay::core::k8090::CommandID::None
    \brief The number of all commands represents also none command.
*/

/*!
    \enum sprelay::core::k8090::ResponseID
    \ingroup group_sprelay_core_public
    \brief Scoped enumeration listing all responses.

    See the Velleman %K8090 card manual.
*/
/*!
    \var sprelay::core::k8090::ResponseID::ButtonMode
    \brief Response with button mode.
*/
/*!
    \var sprelay::core::k8090::ResponseID::Timer
    \brief Response with timer delay.
*/
/*!
    \var sprelay::core::k8090::ResponseID::RelayStatus
    \brief Relay status event.
*/
/*!
    \var sprelay::core::k8090::ResponseID::ButtonStatus
    \brief Button status event.
*/
/*!
    \var sprelay::core::k8090::ResponseID::JumperStatus
    \brief Response with jumper status.
*/
/*!
    \var sprelay::core::k8090::ResponseID::FirmwareVersion
    \brief Response with firmware version.
*/
/*!
    \var sprelay::core::k8090::ResponseID::None
    \brief The number of all responses represents also none response.
*/

/*!
    \enum sprelay::core::k8090::RelayID
    \ingroup group_sprelay_core_public
    \brief Scoped enumeration listing all 8 relays.

    Bitwise operators are enabled for this enum by overloading k8090::enable_bitmask_operators(RelayID) function
    (see enum_flags.h for more details) and so the value of k8090::RelayID type can be also a combination of
    particular relays.
*/
/*!
    \var sprelay::core::k8090::RelayID::None
    \brief None relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::One
    \brief First relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::Two
    \brief Second relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::Three
    \brief Third relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::Four
    \brief Fourth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::Five
    \brief Fifth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::Six
    \brief Sixth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::Seven
    \brief Seventh relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::Eight
    \brief Eigth relay.
*/
/*!
    \var sprelay::core::k8090::RelayID::All
    \brief All relays.
*/

/*!
    \fn constexpr bool sprelay::core::k8090::enable_bitmask_operators(RelayID)
    \ingroup group_sprelay_core_public
    \brief Function overload which enables bitwise operators for RelayID enumeration. See enum_flags.h for more
    details.

    \return True to enable bitmask operators.
*/

/*!
    \fn constexpr RelayID sprelay::core::k8090::from_number(unsigned int number)
    \ingroup group_sprelay_core_public
    \brief Converts number to RelayID scoped enumeration.

    \param number The number.
    \return The RelayID enumerator.
*/

/*!
    \fn constexpr std::underlying_type<E> sprelay::core::k8090::as_number(const E e)
    \ingroup group_sprelay_core_public
    \brief Converts enumeration to its underlying type.

    \param e Enumerator to be converted.
    \return The enum value as underlying type.
*/

#endif  // SPRELAY_CORE_K8090_DEFINES_H_
