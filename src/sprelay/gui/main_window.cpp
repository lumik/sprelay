// -*-c++-*-

/***************************************************************************
**                                                                        **
**  Controlling interface for K8090 8-Channel Relay Card from Velleman    **
**  through usb using virtual serial port in Qt.                          **
**  Copyright (C) 2017 Jakub Klener                                       **
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
 * \file      main_window.cpp
 * \brief     The sprelay::gui::MainWindow class which implements main application window.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2017-03-22
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "main_window.h"

#include <QString>

#include "central_widget.h"


namespace sprelay {
namespace gui {

/*!
 * \brief Constructs the MainWindow
 *
 * The core of application functionality is inside sprelay::gui::CentralWidget which is created inside the MainWindow.
 */
MainWindow::MainWindow()
{
    central_widget_ = new CentralWidget(nullptr, QString(), this);
    setCentralWidget(central_widget_);
}


/*!
 * \brief The destructor.
 */
MainWindow::~MainWindow()
{
}

}  // namespace gui
}  // namespace sprelay
