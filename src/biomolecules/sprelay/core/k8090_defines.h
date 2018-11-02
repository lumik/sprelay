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
 * \ingroup   group_biomolecules_sprelay_core_public
 * \brief     Public defines related to biomolecules::sprelay::core::k8090::K8090 class.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-16
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_CORE_K8090_DEFINES_H_
#define BIOMOLECULES_SPRELAY_CORE_K8090_DEFINES_H_

#include <type_traits>

#include <QMetaType>

#include <lumik/enum_flags/enum_flags.h>

namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {

/// Scoped enumeration listing all commands.
enum struct CommandID : unsigned int
{
    RelayOn,               ///< Switch realy on command.
    RelayOff,              ///< Switch realy off command.
    ToggleRelay,           ///< Toggle realy command.
    QueryRelay,            ///< Query relay status command.
    SetButtonMode,         ///< Set button mode command.
    ButtonMode,            ///< Query button mode command.
    StartTimer,            ///< Start relay timer command.
    SetTimer,              ///< Set relay timer delay command.
    Timer,                 ///< Query timer delay command.
    ResetFactoryDefaults,  ///< Reset factory defaults command.
    JumperStatus,          ///< Jumper status command.
    FirmwareVersion,       ///< Firmware version command.
    None                   ///< The number of all commands represents also none command.
};


/// Scoped enumeration listing all responses.
enum struct ResponseID : unsigned int
{
    ButtonMode,       ///< Response with button mode.
    Timer,            ///< Response with timer delay.
    ButtonStatus,     ///< Button status event.
    RelayStatus,      ///< Relay status event.
    JumperStatus,     ///< Response with jumper status.
    FirmwareVersion,  ///< Response with firmware version.
    None              ///< The number of all responses represents also none response.
};


/// Scoped enumeration listing all 8 relays.
enum struct RelayID : unsigned char
{
    None  = 0,       ///< None relay
    One   = 1 << 0,  ///< First relay.
    Two   = 1 << 1,  ///< Second relay.
    Three = 1 << 2,  ///< Third relay.
    Four  = 1 << 3,  ///< Fourth relay.
    Five  = 1 << 4,  ///< Fifth relay.
    Six   = 1 << 5,  ///< Sixth relay.
    Seven = 1 << 6,  ///< Seventh relay.
    Eight = 1 << 7,  ///< Eigth relay.
    All   = 0xff     ///< All relays.
};


/// Converts number to RelayID scoped enumeration.
constexpr RelayID from_number(unsigned int number) { return static_cast<RelayID>(1 << (number)); }


/// Converts enumeration to its underlying type.
template<typename E>
constexpr typename std::enable_if<std::is_enum<E>::value, std::underlying_type<E>>::type::type as_number(const E e)
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}


}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules


namespace lumik {
namespace enum_flags {

/// \ingroup group_biomolecules_sprelay_core_public
/// \brief Struct specialization which enables bitwise operators for biomolecules::sprelay::core::k8090::RelayID
/// enumeration. See the \ref group_enum_flags "enum_flags" documentation for more details.
template<>
struct EnableBitmaskOperators<biomolecules::sprelay::core::k8090::RelayID> {
    static constexpr bool value = true;  ///< The `true` value enables bitmask operators on
                                         ///< biomolecules::sprelay::core::k8090::RelayID
};

}  // namespace enum_flags
}  // namespace lumik


Q_DECLARE_METATYPE(biomolecules::sprelay::core::k8090::RelayID)

/*!
 * \enum biomolecules::sprelay::core::k8090::CommandID
 * \ingroup group_biomolecules_sprelay_core_public
 *
 * See the Velleman %K8090 card manual.
 */

/*!
 * \enum biomolecules::sprelay::core::k8090::ResponseID
 * \ingroup group_biomolecules_sprelay_core_public
 *
 * See the Velleman %K8090 card manual.
 */

/*!
 * \enum biomolecules::sprelay::core::k8090::RelayID
 * \ingroup group_biomolecules_sprelay_core_public
 *
 * Bitwise operators are enabled for this enum by specializing
 * lumik::enum_flags::EnableBitmaskOperators<biomolecules::sprelay::core::k8090::RelayID> structure
 * (see `enum_flags` documentation for more details) and so the value of k8090::RelayID type can be also a combination
 * of particular relays.
 */

/*!
 * \fn constexpr RelayID biomolecules::sprelay::core::k8090::from_number(unsigned int number)
 * \ingroup group_biomolecules_sprelay_core_public
 * \param number The number.
 * \return The RelayID enumerator.
 */

/*!
 * \fn constexpr std::underlying_type<E> biomolecules::sprelay::core::k8090::as_number(const E e)
 * \ingroup group_biomolecules_sprelay_core_public
 * \param e Enumerator to be converted.
 * \return The enum value as underlying type.
 */

#endif  // BIOMOLECULES_SPRELAY_CORE_K8090_DEFINES_H_
