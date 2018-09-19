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

#ifndef SPRELAY_CORE_SERIAL_PORT_UTILS_H_
#define SPRELAY_CORE_SERIAL_PORT_UTILS_H_

#include <QString>

#include <memory>

namespace sprelay {
namespace core {

// forwad declarations
class MockSerialPort;

namespace serial_utils {

bool hex_to_byte(const QString &msg, std::unique_ptr<unsigned char[]> *buffer, int *n);
QString byte_to_hex(const unsigned char *buffer, int n);

// specify deleter to enbale forward declarations
struct MockSerialPortDeleter
{
    void operator()(MockSerialPort *p);
};

}  // namespace serial_utils
}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_SERIAL_PORT_UTILS_H_
