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
 * \file      core_test_utils.h
 * \brief     Utilities for sprelay core tests.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-03-07
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include <QMetaType>

#ifndef BIOMOLECULES_SPRELAY_CORE_CORE_TEST_UTILS_H_
#define BIOMOLECULES_SPRELAY_CORE_CORE_TEST_UTILS_H_

// enables usage of const unsigned char* pointer in signals and slots
Q_DECLARE_METATYPE(unsigned char)
Q_DECLARE_METATYPE(const unsigned char *)

#endif  // BIOMOLECULES_SPRELAY_CORE_CORE_TEST_UTILS_H_

