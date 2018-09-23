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
 * \file      serial_port_utils_test.h
 * \brief     The sprelay::core::SerialPortUtilsTest class which implements tests for serial port utilities.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_CORE_SERIAL_PORT_UTILS_TEST_H_
#define SPRELAY_CORE_SERIAL_PORT_UTILS_TEST_H_

#include <QObject>

#include "lumik/qtest_suite/qtest_suite.h"

namespace sprelay {
namespace core {
namespace serial_utils {

class SerialPortUtilsTest : public QObject
{
    Q_OBJECT
private slots:  // NOLINT(whitespace/indent)
    void hexToByteTestCase();
};

ADD_TEST(SerialPortUtilsTest)

}  // namespace serial_utils
}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_SERIAL_PORT_UTILS_TEST_H_
