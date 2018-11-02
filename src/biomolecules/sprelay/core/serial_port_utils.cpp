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
 * \file      serial_port_utils.cpp
 * \brief     Utility functions and data structures for biomolecules::sprelay::core::UnifiedSerialPort and
 *            biomolecules::sprelay::core::MockSerialPort.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "serial_port_utils.h"

#include <utility>

#include "mock_serial_port.h"

namespace biomolecules {
namespace sprelay {
namespace core {
namespace serial_utils {

/*!
 * \param msg String message representation. It has to be in format `00 A5 4B`...
 * \param buffer Pointer where the binary message will be stored.
 * \param n Pointer to variable, where the number of bytes will be stored.
 * \return True if the conversion was successful.
 */
bool hex_to_byte(const QString &msg, std::unique_ptr<unsigned char[]> *buffer, int *n)
{
    // remove white spaces
    QString newMsg = msg;
    newMsg.remove(' ');

    int msgSize = newMsg.size();

    // test correct size of msg, all hex codes consit of 2 characters
    if (msgSize % 2) {
        *n = 0;
        // TODO(lumik): change to exception
        return false;
    }
    *n = msgSize / 2;
    buffer->reset(new unsigned char[*n]);
    // TODO(lumik): change to exception
    bool ok;
    for (int ii = 0; ii < *n; ++ii) {
        (*buffer)[ii] = newMsg.midRef(2 * ii, 2).toUInt(&ok, 16);
    }

    return ok;
}


/*!
 * \param buffer The buffer with bytes to be converted.
 * \param n The number of bytes to be converted.
 * \return The string representation of the buffer.
 */
QString byte_to_hex(const unsigned char *buffer, int n)
{
    QString msg;
    for (int ii = 0; ii < n - 1; ++ii) {
        msg.append(QString("%1").arg((unsigned int)buffer[ii], 2, 16, QChar('0')).toUpper()).append(' ');
    }
    if (n > 0) {
        msg.append(QString("%1").arg((unsigned int)buffer[n - 1], 2, 16, QChar('0')).toUpper());
    }
    return msg;
}


/*!
 * \struct biomolecules::sprelay::core::serial_utils::MockSerialPortDeleter
 * Example:
 * \code
 * #include <memory>
 *
 * #include "serial_port_utils.h"
 *
 * namespace sprelay {
 * namespace core {
 * namespace serial_utils {
 *
 * // forward declarations
 * class MockSerialPort;
 *
 * }  // namespace serial_utils
 * }  // namespace core
 * }  // namespace sprelay
 *
 * struct MyStruct {
 *     std::unique_ptr<biomolecules::sprelay::core::MockSerialPort,
 *             biomolecules::sprelay::core::serial_utils::MockSerialPortDeleter>
 *         mock_serial_port_;
 * };
 * \endcode
 */

/*!
 * \brief Deletes the object.
 *
 * \param p Pointer to object to be deleted.
 */
void MockSerialPortDeleter::operator()(MockSerialPort *p)
{
    delete p;
}

}  // namespace serial_utils
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
