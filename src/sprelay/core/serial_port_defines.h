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
 * \file      serial_port_defines.h
 * \brief     Public defines concerning sprelay::core::UnifiedSerialPort.

 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-09
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_CORE_SERIAL_PORT_DEFINES_H_
#define SPRELAY_CORE_SERIAL_PORT_DEFINES_H_

#include <QString>

namespace sprelay {
namespace core {
namespace serial_utils {

/*!
 * \struct ComPortParams
 * \brief Structure containing information about COM port.
 */
struct ComPortParams
{
    QString port_name;
    QString description;
    QString manufacturer;
    quint16 product_identifier;
    quint16 vendor_identifier;
};

}  // namespace serial_utils
}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_SERIAL_PORT_DEFINES_H_
