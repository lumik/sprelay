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
 * \file      k8090_commands.h
 * \brief     Command binary representations and other constants for Veleman %K8090 card.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-16
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */

#ifndef BIOMOLECULES_SPRELAY_CORE_K8090_COMMANDS_H_
#define BIOMOLECULES_SPRELAY_CORE_K8090_COMMANDS_H_

#include <array>

#include "k8090_defines.h"

namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {
namespace impl_ {

// generate static arrays containing commands, command priorities and responses at compile time


// tried to make the static array creation more modern but gcc 4.9 does not treat constexpr correctly
// so revert to the old way.
// // generator functions
//constexpr unsigned char make_binary_command(std::size_t id) noexcept
//{
//    return
//        // clang-format off
//        id == as_number(CommandID::RelayOn) ?  0x11 :
//        id == as_number(CommandID::RelayOff) ? 0x12 :
//        id == as_number(CommandID::ToggleRelay) ? 0x14 :
//        id == as_number(CommandID::QueryRelay) ? 0x18 :
//        id == as_number(CommandID::SetButtonMode) ? 0x21 :
//        id == as_number(CommandID::ButtonMode) ? 0x22 :
//        id == as_number(CommandID::StartTimer) ? 0x41 :
//        id == as_number(CommandID::SetTimer) ? 0x42 :
//        id == as_number(CommandID::Timer) ? 0x44 :
//        id == as_number(CommandID::ResetFactoryDefaults) ? 0x66 :
//        id == as_number(CommandID::JumperStatus) ? 0x70 :
//        id == as_number(CommandID::FirmwareVersion) ? 0x71 :
//        0;
//    // clang-format on
//}

//constexpr int make_command_priority(std::size_t id) noexcept
//{
//    return
//        // clang-format off
//        id == as_number(CommandID::RelayOn) ? 1 :
//        id == as_number(CommandID::RelayOff) ? 1 :
//        id == as_number(CommandID::ToggleRelay) ? 1 :
//        id == as_number(CommandID::QueryRelay) ? 2 :
//        id == as_number(CommandID::SetButtonMode) ? 1 :
//        id == as_number(CommandID::ButtonMode) ? 2 :
//        id == as_number(CommandID::StartTimer) ? 1 :
//        id == as_number(CommandID::SetTimer) ? 1 :
//        id == as_number(CommandID::Timer) ? 2 :
//        id == as_number(CommandID::ResetFactoryDefaults) ? 1 :
//        id == as_number(CommandID::JumperStatus) ? 1 :
//        id == as_number(CommandID::FirmwareVersion) ? 1 :
//        0;
//    // clang-format on
//}

//constexpr unsigned char make_binary_response(std::size_t id) noexcept
//{
//    return
//        // clang-format off
//        id == as_number(ResponseID::ButtonMode) ? 0x22 :
//        id == as_number(ResponseID::Timer) ? 0x44 :
//        id == as_number(ResponseID::ButtonStatus) ? 0x50 :
//        id == as_number(ResponseID::RelayStatus) ? 0x51 :
//        id == as_number(ResponseID::JumperStatus) ? 0x70 :
//        id == as_number(ResponseID::FirmwareVersion) ? 0x71 :
//        0;
//    // clang-format on
//}

// // helper structures

//template<std::size_t...>
//struct IndexSequence
//{};

//template<std::size_t tCounter, std::size_t... tRest>
//struct IndexSequenceGenerator : IndexSequenceGenerator<tCounter - 1, tCounter - 1, tRest...>
//{};

// // zero case
//template<std::size_t... tRest>
//struct IndexSequenceGenerator<0u, tRest...>
//{
//    using type = IndexSequence<tRest...>;
//};

//template<std::size_t N>
//using MakeIndexSequence = typename IndexSequenceGenerator<N>::type;

//template<typename T, std::size_t... tIndexes, std::size_t tSize = sizeof...(tIndexes)>
//constexpr std::array<T, tSize> make_value_array_from_seq(
//    T (*func)(std::size_t), IndexSequence<tIndexes...> /*unused*/) noexcept
//{
//    return {func(tIndexes)...};
//}

// // functions that make the arrays
//constexpr std::array<unsigned char, as_number(CommandID::None)> make_command_array() noexcept
//{
//    return make_value_array_from_seq(&make_binary_command, MakeIndexSequence<as_number(CommandID::None)>{});
//}

//constexpr std::array<int, as_number(CommandID::None)> make_command_priority_array() noexcept
//{
//    return make_value_array_from_seq(&make_command_priority, MakeIndexSequence<as_number(CommandID::None)>{});
//}

//constexpr std::array<unsigned char, as_number(ResponseID::None)> make_response_array() noexcept
//{
//    return make_value_array_from_seq(&make_binary_response, MakeIndexSequence<as_number(ResponseID::None)>{});
//}

// template function to fill the array with appropriate commands and priorities
template<unsigned int N>
struct CommandDataValue;

// specializations
template<>
struct CommandDataValue<as_number(CommandID::RelayOn)>
{
    static const unsigned char kCommand = 0x11;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::RelayOff)>
{
    static const unsigned char kCommand = 0x12;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::ToggleRelay)>
{
    static const unsigned char kCommand = 0x14;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::QueryRelay)>
{
    static const unsigned char kCommand = 0x18;
    static const int kPriority = 2;
};
template<>
struct CommandDataValue<as_number(CommandID::SetButtonMode)>
{
    static const unsigned char kCommand = 0x21;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::ButtonMode)>
{
    static const unsigned char kCommand = 0x22;
    static const int kPriority = 2;
};
template<>
struct CommandDataValue<as_number(CommandID::StartTimer)>
{
    static const unsigned char kCommand = 0x41;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::SetTimer)>
{
    static const unsigned char kCommand = 0x42;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::Timer)>
{
    static const unsigned char kCommand = 0x44;
    static const int kPriority = 2;
};
template<>
struct CommandDataValue<as_number(CommandID::ResetFactoryDefaults)>
{
    static const unsigned char kCommand = 0x66;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::JumperStatus)>
{
    static const unsigned char kCommand = 0x70;
    static const int kPriority = 1;
};
template<>
struct CommandDataValue<as_number(CommandID::FirmwareVersion)>
{
    static const unsigned char kCommand = 0x71;
    static const int kPriority = 1;
};


// template function to fill the array with appropriate responses
template<unsigned int N>
struct ResponseDataValue;

// specializations
template<>
struct ResponseDataValue<as_number(ResponseID::ButtonMode)>
{
    static const unsigned char kCommand = 0x22;
};
template<>
struct ResponseDataValue<as_number(ResponseID::Timer)>
{
    static const unsigned char kCommand = 0x44;
};
template<>
struct ResponseDataValue<as_number(ResponseID::ButtonStatus)>
{
    static const unsigned char kCommand = 0x50;
};
template<>
struct ResponseDataValue<as_number(ResponseID::RelayStatus)>
{
    static const unsigned char kCommand = 0x51;
};
template<>
struct ResponseDataValue<as_number(ResponseID::JumperStatus)>
{
    static const unsigned char kCommand = 0x70;
};
template<>
struct ResponseDataValue<as_number(ResponseID::FirmwareVersion)>
{
    static const unsigned char kCommand = 0x71;
};

// Template containing static array
template<typename T, T... Args>
struct XArrayData
{
    // initializing declaration
    static constexpr std::array<T, sizeof...(Args)> kValues{Args...};
};

// recursively generates command typedefs
template<unsigned int N, unsigned char... Args>
struct CommandArrayGenerator
{
    using Commands = typename CommandArrayGenerator<N - 1, CommandDataValue<N - 1>::kCommand, Args...>::Commands;
    using Priorities = typename CommandArrayGenerator<N - 1, CommandDataValue<N - 1>::kPriority, Args...>::Priorities;
};

// end case template partial specialization of command typedefs
template<unsigned char... Args>
struct CommandArrayGenerator<1u, Args...>
{
    using Commands = XArrayData<unsigned char, CommandDataValue<0u>::kCommand, Args...>;
    using Priorities = XArrayData<int, CommandDataValue<0u>::kPriority, Args...>;
};

// CommandArray generates recursively kCommands nad kPriorities types, which contains static constant array kValues.
// Usage: unsigned char* arr = CommandArray<k8090::Comand::None>::kCommands::kValues
template<unsigned char N>
struct CommandArray_
{
    using Commands = typename CommandArrayGenerator<N>::Commands;
    using Priorities = typename CommandArrayGenerator<N>::Priorities;
};

// recursively generates reponse typedefs
template<unsigned int N, unsigned char... Args>
struct ResponseArrayGenerator
{
    using Responses = typename ResponseArrayGenerator<N - 1, ResponseDataValue<N - 1>::kCommand, Args...>::Responses;
};

// end case template partial specialization of response typedefs
template<unsigned char... Args>
struct ResponseArrayGenerator<1u, Args...>
{
    using Responses = XArrayData<unsigned char, ResponseDataValue<0u>::kCommand, Args...>;
};

// ResponseArray generates recursively kResponses type, which contains static constant array kValues.
// Usage: unsigned char* arr = ResponseArray<k8090::Comand::None>::kCommands::kValues
template<unsigned char N>
struct ResponseArray_
{
    using Responses = typename ResponseArrayGenerator<N>::Responses;
};

// static const array definition (needed to create the static array kValues to satisfy ODR, deprecated c++17)
template<typename T, T... Args>
constexpr std::array<T, sizeof...(Args)> XArrayData<T, Args...>::kValues;

/*!
 * \brief Array of hexadecimal representation of commands used to control the relay.
 */
constexpr std::array<unsigned char, as_number(CommandID::None)> kCommands =
    CommandArray_<as_number(CommandID::None)>::Commands::kValues;

/*!
 * \brief Array of default priorities used to command scheduling.
 */
constexpr std::array<int, as_number(CommandID::None)> kPriorities =
    CommandArray_<as_number(CommandID::None)>::Priorities::kValues;

/*!
 * \brief Array of hexadecimal representation of responses sended by the relay.
 */
constexpr std::array<unsigned char, as_number(ResponseID::None)> kResponses =
    ResponseArray_<as_number(ResponseID::None)>::Responses::kValues;

/*!
 * \brief Start delimiting command byte.
 */
constexpr unsigned char kStxByte = 0x04;

/*!
 * \brief End delimiting command byte.
 */

constexpr unsigned char kEtxByte = 0x0f;

/*!
 * \brief Product id for the automatic port identification.
 */
const quint16 kProductID = 32912;
/*!
 * \brief Vendor id for the automatic port identification.
 */
const quint16 kVendorID = 4303;

const char* const kMockPortName = "MOCKCOM";

}  // namespace impl_
}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPRELAY_CORE_K8090_COMMANDS_H_
