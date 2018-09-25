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
 * \file      serial_port_utils.h
 * \brief     Utility functions and data structures for sprelay::core::UnifiedSerialPort and
 *            sprelay::core::MockSerialPort.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_CORE_SERIAL_PORT_UTILS_H_
#define SPRELAY_CORE_SERIAL_PORT_UTILS_H_

#include <QString>

#include <memory>

namespace sprelay {
namespace core {

// forwad declarations
class MockSerialPort;

namespace serial_utils {

/// Converts hexadecimal string message to its binary representation.
bool hex_to_byte(const QString &msg, std::unique_ptr<unsigned char[]> *buffer, int *n);

/// Converts binary message to its string representation.
QString byte_to_hex(const unsigned char *buffer, int n);

/// \brief Deleter needed for forward declaration of sprelay::core::MockSerialPort class.
/// \headerfile ""
struct MockSerialPortDeleter
{
    void operator()(MockSerialPort *p);
};

}  // namespace serial_utils
}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_SERIAL_PORT_UTILS_H_
