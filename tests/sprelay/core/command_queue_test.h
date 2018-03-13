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

#ifndef SPRELAY_CORE_COMMAND_QUEUE_TEST_H_
#define SPRELAY_CORE_COMMAND_QUEUE_TEST_H_

#include <QObject>

namespace sprelay {
namespace core {
namespace command_queue {

class CommandQueueTest: public QObject
{
    Q_OBJECT
private slots:  // NOLINT(whitespace/indent)
    void uniquePush();
    void notUniquePush();
    void updateCommand();
};

}  // namespace command_queue
}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_COMMAND_QUEUE_TEST_H_

