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
 * \ingroup   group_biomolecules_sprelay_core_public
 * \brief     Public defines concerning biomolecules::sprelay::core::UnifiedSerialPort.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-09
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_CORE_SERIAL_PORT_DEFINES_H_
#define BIOMOLECULES_SPRELAY_CORE_SERIAL_PORT_DEFINES_H_

#include <QString>

namespace biomolecules {
namespace sprelay {
namespace core {
namespace serial_utils {

/// Structure representing informations about one serial port. Used by the UnifiedSerialPort::availablePorts() method.
struct ComPortParams
{
    QString port_name;           ///< Port name.
    QString description;         ///< Port description.
    QString manufacturer;        ///< Port manufacturer.
    quint16 product_identifier;  ///< Port product identifier.
    quint16 vendor_identifier;   ///< Port vendor identifier.
};

}  // namespace serial_utils
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

/*!
 * \struct biomolecules::sprelay::core::serial_utils::ComPortParams
 * \ingroup group_biomolecules_sprelay_core_public
 */

#endif  // BIOMOLECULES_SPRELAY_CORE_SERIAL_PORT_DEFINES_H_
