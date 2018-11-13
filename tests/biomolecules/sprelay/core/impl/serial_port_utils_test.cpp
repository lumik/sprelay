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
 * \file      serial_port_utils_test.cpp
 * \brief     The biomolecules::sprelay::core::SerialPortUtilsTest class which implements tests for serial port
 *            utilities.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "serial_port_utils_test.h"

#include <QtTest>

#include <memory>

#include "biomolecules/sprelay/core/serial_port_utils.h"

namespace biomolecules {
namespace sprelay {
namespace core {
namespace serial_utils {

void SerialPortUtilsTest::hexToByteTestCase()
{
    const int expected_n = 3;

    // testing message
    unsigned char nMsg[expected_n] = {0x1, 0xFF, 0xF};
    QString msg = "01 FF 0F";

    int n;
    bool ok;
    std::unique_ptr<unsigned char[]> bMsg = hex_to_byte(msg, &n, &ok);
    QCOMPARE(n, expected_n);
    QCOMPARE(ok, true);

    for (int i = 0; i < expected_n; i++) {
        if (bMsg[i] != nMsg[i]) {
            ok = false;
        }
    }
    QCOMPARE(ok, true);
}

}  // namespace serial_utils
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
