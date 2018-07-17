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
 * \file      k8090_utils_test.cpp
 * \brief     Tests for utility functions and data structures for K8090 class implementation.

 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-17
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "k8090_utils_test.h"

#include <QByteArray>
#include <QString>
#include <QtTest>

#include "sprelay/core/k8090_defines.h"
#include "sprelay/core/k8090_utils.h"

Q_DECLARE_METATYPE(sprelay::core::k8090::impl_::Command)

namespace sprelay {
namespace core {
namespace k8090 {
namespace impl_ {

void K8090UtilsTest::checkSum()
{
    const int n = 5;
    const unsigned char message[n] = {0x04, 0x22, 0x10, 0xcf, 0x20};
    const unsigned char response = 0xdb;
    QCOMPARE(check_sum(message, n), response);
}


void CommandTest::orEqual_data()
{
    QTest::addColumn<Command>("command1");
    QTest::addColumn<Command>("command2");
    QTest::addColumn<Command>("result");

    const int priority1 = 1;
    const int priority2 = 2;

    // relay on command with random parameters
    {
        const Command cmd1{CommandID::RELAY_ON, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority2, 2u, 3u, 4u};
        const Command response{CommandID::RELAY_ON, priority1, 1u | 2u, 2u, 3u};
        QTest::newRow("Random relay on") << cmd1 << cmd2 << response;
    }

    // duplicated relay on
    {
        const Command cmd1{CommandID::RELAY_ON, priority2, as_number(RelayID::ONE | RelayID::TWO), 5u, 10u};
        const Command cmd2{CommandID::RELAY_ON, priority1, as_number(RelayID::TWO | RelayID::THREE), 10u, 5u};
        const Command response{CommandID::RELAY_ON, priority2, as_number(RelayID::ONE | RelayID::TWO | RelayID::THREE),
            5u, 10u};
        QTest::newRow("Duplicated relay on") << cmd1 << cmd2 << response;
    }

    // relay on with relay off command
    {
        const Command cmd1{CommandID::RELAY_ON, priority2, as_number(RelayID::ONE | RelayID::TWO), 0, 0};
        const Command cmd2{CommandID::RELAY_OFF, priority1, as_number(RelayID::TWO | RelayID::THREE), 0, 0};
        const Command response{CommandID::RELAY_ON, priority2, as_number(RelayID::ONE), 0, 0};
        QTest::newRow("Relay on + off") << cmd1 << cmd2 << response;
    }

    // relay on with random command
    {
        const Command cmd1{CommandID::RELAY_ON, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::NONE, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response{CommandID::RELAY_ON, priority2, as_number(RelayID::ONE | RelayID::TWO | RelayID::THREE),
            3u, 7u};
        QTest::newRow("Relay on + none") << cmd1 << cmd2 << response;
    }

    // relay off
    {
        const Command cmd1{CommandID::RELAY_OFF, priority1, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::RELAY_OFF, priority2, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response{CommandID::RELAY_OFF, priority1, as_number(RelayID::ONE | RelayID::TWO | RelayID::THREE),
            3u, 7u};
        QTest::newRow("Relay off") << cmd1 << cmd2 << response;
    }

    // relay off with relay on command
    {
        const Command cmd1{CommandID::RELAY_OFF, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::RELAY_ON, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response{CommandID::RELAY_OFF, priority2, as_number(RelayID::ONE), 3u, 7u};
        QTest::newRow("Relay off + on") << cmd1 << cmd2 << response;
    }

    // toggle relay command
    {
        const Command cmd1{CommandID::TOGGLE_RELAY, priority1, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::TOGGLE_RELAY, priority2, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response{CommandID::TOGGLE_RELAY, priority1, as_number(RelayID::ONE | RelayID::THREE), 3u, 7u};
        QTest::newRow("toggle") << cmd1 << cmd2 << response;
    }

    // set button mode
    {
        const unsigned char momentary1 = as_number(RelayID::ONE | RelayID::TWO | RelayID::THREE);
        const unsigned char toggle1 = as_number(RelayID::FOUR | RelayID::FIVE | RelayID::SIX);
        const unsigned char timed1 = as_number(RelayID::SEVEN | RelayID::EIGHT);
        const unsigned char momentary2 = as_number(RelayID::FOUR | RelayID::SEVEN);
        const unsigned char toggle2 = as_number(RelayID::ONE | RelayID::THREE | RelayID::FIVE);
        const unsigned char timed2 = as_number(RelayID::TWO | RelayID::SIX | RelayID::EIGHT);
        const Command cmd1{CommandID::SET_BUTTON_MODE, priority2, momentary1, toggle1, timed1};
        const Command cmd2{CommandID::SET_BUTTON_MODE, priority1, momentary2, toggle2, timed2};
        const Command response{CommandID::SET_BUTTON_MODE, priority2, momentary1 | momentary2,
            as_number(RelayID::FIVE | RelayID::SIX), as_number(RelayID::EIGHT)};
        QTest::newRow("set button mode") << cmd1 << cmd2 << response;
    }

    // start timer
    {
        const Command cmd1{CommandID::START_TIMER, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::START_TIMER, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response{CommandID::START_TIMER, priority2,
            as_number(RelayID::ONE | RelayID::TWO | RelayID::THREE), 3u, 7u};
        QTest::newRow("start timer") << cmd1 << cmd2 << response;
    }

    // set timer
    {
        const Command cmd1{CommandID::SET_TIMER, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::SET_TIMER, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response{CommandID::SET_TIMER, priority2, as_number(RelayID::ONE | RelayID::TWO | RelayID::THREE),
            3u, 7u};
        QTest::newRow("set timer") << cmd1 << cmd2 << response;
    }

    // query timer
    {
        const Command cmd1{CommandID::TIMER, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::TIMER, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response{CommandID::TIMER, priority2, as_number(RelayID::ONE | RelayID::TWO | RelayID::THREE), 3u,
            7u};
        QTest::newRow("query timer") << cmd1 << cmd2 << response;
    }

    // query relay status
    {
        const Command cmd1{CommandID::QUERY_RELAY, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::QUERY_RELAY, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response = cmd1;
        QTest::newRow("query relay status") << cmd1 << cmd2 << response;
    }

    // query button modes
    {
        const Command cmd1{CommandID::BUTTON_MODE, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::BUTTON_MODE, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response = cmd1;
        QTest::newRow("query button modes") << cmd1 << cmd2 << response;
    }

    // reset factory defaults
    {
        const Command cmd1{CommandID::RESET_FACTORY_DEFAULTS, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u,
            7u};
        const Command cmd2{CommandID::RESET_FACTORY_DEFAULTS, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u,
            3u};
        const Command response = cmd1;
        QTest::newRow("reset factory defaults") << cmd1 << cmd2 << response;
    }

    // jumper status
    {
        const Command cmd1{CommandID::JUMPER_STATUS, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::JUMPER_STATUS, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response = cmd1;
        QTest::newRow("jumper status") << cmd1 << cmd2 << response;
    }

    // firmware version
    {
        const Command cmd1{CommandID::FIRMWARE_VERSION, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::FIRMWARE_VERSION, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response = cmd1;
        QTest::newRow("firmware version") << cmd1 << cmd2 << response;
    }

    // none command
    {
        const Command cmd1{CommandID::NONE, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::NONE, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response = cmd1;
        QTest::newRow("none command") << cmd1 << cmd2 << response;
    }

    // none command with relay on
    {
        const Command cmd1{CommandID::NONE, priority2, as_number(RelayID::ONE | RelayID::TWO), 3u, 7u};
        const Command cmd2{CommandID::RELAY_ON, priority1, as_number(RelayID::TWO | RelayID::THREE), 5u, 3u};
        const Command response = cmd1;
        QTest::newRow("none command + relay on") << cmd1 << cmd2 << response;
    }
}


void CommandTest::orEqual()
{
    // fetch data
    QFETCH(Command, command1);
    QFETCH(Command, command2);
    QFETCH(Command, result);

    // merge commands
    command1 |= command2;

    // test result
    QVERIFY2(command1.id == result.id,
        qPrintable(QString{"id = '%1' does not match the expected %2."}
            .arg(as_number(command1.id)).arg(as_number(result.id))));
    QCOMPARE(command1.priority, result.priority);
    for (int i = 0; i < 3; ++i) {
        QVERIFY2(command1.params[i] == result.params[i],
            qPrintable(QString{"params[%1] = '%2' does not match the expected %3."}
                .arg(i).arg(command1.params[i], 8, 2, QChar('0')).arg(result.params[i], 8, 2, QChar('0'))));
    }
}


void CommandTest::equality_data()
{
    QTest::addColumn<Command>("command1");
    QTest::addColumn<Command>("command2");

    const int priority1 = 1;
    const int priority2 = 2;

    // commands with the same priority
    {
        const Command cmd1{CommandID::RELAY_ON, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority1, 1u, 2u, 3u};
        QTest::newRow("the same priority") << cmd1 << cmd2;
    }

    // commands with different priorities
    {
        const Command cmd1{CommandID::RELAY_ON, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority2, 1u, 2u, 3u};
        QTest::newRow("different priorities") << cmd1 << cmd2;
    }
}


void CommandTest::equality()
{
    // fetch data
    QFETCH(Command, command1);
    QFETCH(Command, command2);

    // equality
    QCOMPARE(command1, command2);
    // not equality
    QVERIFY(!(command1 != command2));
}


void CommandTest::equalityFalse_data()
{
    QTest::addColumn<Command>("command1");
    QTest::addColumn<Command>("command2");

    const int priority = 1;

    // different ids
    {
        const Command cmd1{CommandID::RELAY_ON, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_OFF, priority, 1u, 2u, 3u};
        QTest::newRow("different ids") << cmd1 << cmd2;
    }

    // different 1st param
    {
        const Command cmd1{CommandID::RELAY_ON, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority, 2u, 2u, 3u};
        QTest::newRow("different 1st param") << cmd1 << cmd2;
    }

    // different 2nd param
    {
        const Command cmd1{CommandID::RELAY_ON, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority, 1u, 3u, 3u};
        QTest::newRow("different 2nd param") << cmd1 << cmd2;
    }

    // different 3rd param
    {
        const Command cmd1{CommandID::RELAY_ON, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority, 1u, 2u, 4u};
        QTest::newRow("different 3rd param") << cmd1 << cmd2;
    }
}


void CommandTest::equalityFalse()
{
    // fetch data
    QFETCH(Command, command1);
    QFETCH(Command, command2);

    // equality
    QVERIFY(!(command1 == command2));
    // not equality
    QVERIFY(command1 != command2);
}


void CommandTest::isCompatible_data()
{
    QTest::addColumn<Command>("command1");
    QTest::addColumn<Command>("command2");

    const int priority1 = 1;
    const int priority2 = 2;

    // The same commands
    {
        const Command cmd1{CommandID::TOGGLE_RELAY, priority1, 1u, 2u, 3u};
        const Command cmd2 = cmd1;
        QTest::newRow("the same command") << cmd1 << cmd2;
    }

    // relay on
    {
        const Command cmd1{CommandID::RELAY_ON, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority2, 2u, 3u, 4u};
        QTest::newRow("relay on") << cmd1 << cmd2;
    }

    // relay on + off
    {
        const Command cmd1{CommandID::RELAY_ON, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_OFF, priority2, 2u, 3u, 4u};
        QTest::newRow("relay on + off") << cmd1 << cmd2;
    }

    // relay off
    {
        const Command cmd1{CommandID::RELAY_OFF, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_OFF, priority2, 2u, 3u, 4u};
        QTest::newRow("relay off") << cmd1 << cmd2;
    }

    // relay off + on
    {
        const Command cmd1{CommandID::RELAY_OFF, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RELAY_ON, priority2, 2u, 3u, 4u};
        QTest::newRow("relay off + on") << cmd1 << cmd2;
    }

    // start timer
    {
        const Command cmd1{CommandID::START_TIMER, priority1, as_number(RelayID::ONE | RelayID::TWO), 2u, 3u};
        const Command cmd2{CommandID::START_TIMER, priority2, as_number(RelayID::TWO | RelayID::THREE), 2u, 3u};
        QTest::newRow("start timer") << cmd1 << cmd2;
    }

    // set timer
    {
        const Command cmd1{CommandID::SET_TIMER, priority1, as_number(RelayID::ONE | RelayID::TWO), 2u, 3u};
        const Command cmd2{CommandID::SET_TIMER, priority2, as_number(RelayID::TWO | RelayID::THREE), 2u, 3u};
        QTest::newRow("set timer") << cmd1 << cmd2;
    }

    // query total timer
    {
        const Command cmd1{CommandID::TIMER, priority1, as_number(RelayID::ONE | RelayID::TWO),
            as_number(TimerDelayType::TOTAL), 3u};
        const Command cmd2{CommandID::TIMER, priority2, as_number(RelayID::TWO | RelayID::THREE),
            as_number(TimerDelayType::TOTAL), 4u};
        QTest::newRow("query total timer") << cmd1 << cmd2;
    }

    // query remaining timer
    {
        const Command cmd1{CommandID::TIMER, priority1, as_number(RelayID::ONE | RelayID::TWO),
            as_number(TimerDelayType::REMAINING), 3u};
        const Command cmd2{CommandID::TIMER, priority2, as_number(RelayID::TWO | RelayID::THREE),
            as_number(TimerDelayType::REMAINING), 4u};
        QTest::newRow("query remaining timer") << cmd1 << cmd2;
    }

    // toggle relay
    {
        const Command cmd1{CommandID::TOGGLE_RELAY, priority1, as_number(RelayID::ONE | RelayID::TWO), 2u, 3u};
        const Command cmd2{CommandID::TOGGLE_RELAY, priority2, as_number(RelayID::TWO | RelayID::THREE), 3u, 4u};
        QTest::newRow("toggle relay") << cmd1 << cmd2;
    }

    // query relay status
    {
        const Command cmd1{CommandID::QUERY_RELAY, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::QUERY_RELAY, priority2, 2u, 3u, 4u};
        QTest::newRow("query relay status") << cmd1 << cmd2;
    }

    // set button mode
    {
        const Command cmd1{CommandID::SET_BUTTON_MODE, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::SET_BUTTON_MODE, priority2, 2u, 3u, 4u};
        QTest::newRow("set button mode") << cmd1 << cmd2;
    }

    // query button mode
    {
        const Command cmd1{CommandID::BUTTON_MODE, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::BUTTON_MODE, priority2, 2u, 3u, 4u};
        QTest::newRow("query button mode") << cmd1 << cmd2;
    }

    // reset factory defaults
    {
        const Command cmd1{CommandID::RESET_FACTORY_DEFAULTS, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::RESET_FACTORY_DEFAULTS, priority2, 2u, 3u, 4u};
        QTest::newRow("reset factory defaults") << cmd1 << cmd2;
    }

    // jumper status
    {
        const Command cmd1{CommandID::JUMPER_STATUS, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::JUMPER_STATUS, priority2, 2u, 3u, 4u};
        QTest::newRow("jumper status") << cmd1 << cmd2;
    }

    // firmware version
    {
        const Command cmd1{CommandID::FIRMWARE_VERSION, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::FIRMWARE_VERSION, priority2, 2u, 3u, 4u};
        QTest::newRow("firmware version") << cmd1 << cmd2;
    }

    // none command
    {
        const Command cmd1{CommandID::NONE, priority1, 1u, 2u, 3u};
        const Command cmd2{CommandID::NONE, priority2, 2u, 3u, 4u};
        QTest::newRow("none command") << cmd1 << cmd2;
    }
}


void CommandTest::isCompatible()
{
    // fetch data
    QFETCH(Command, command1);
    QFETCH(Command, command2);

    // test
    QVERIFY(command1.isCompatible(command2));
}


void CommandTest::isNotCompatible_data()
{
    QTest::addColumn<Command>("command1");
    QTest::addColumn<Command>("command2");

    const int priority = 1;

    // relay on
    {
        const Command cmd1{CommandID::RELAY_ON, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::TOGGLE_RELAY, priority, 1u, 2u, 3u};
        QTest::newRow("relay on") << cmd1 << cmd2;
    }

    // relay off
    {
        const Command cmd1{CommandID::RELAY_OFF, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::TOGGLE_RELAY, priority, 1u, 2u, 3u};
        QTest::newRow("relay off") << cmd1 << cmd2;
    }

    // start timer
    {
        const Command cmd1{CommandID::START_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        const Command cmd2{CommandID::SET_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        QTest::newRow("start timer") << cmd1 << cmd2;
    }

    // start timer - 1st arg
    {
        const Command cmd1{CommandID::START_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        const Command cmd2{CommandID::START_TIMER, priority, as_number(RelayID::ONE), 3u, 3u};
        QTest::newRow("start timer - 1st arg") << cmd1 << cmd2;
    }

    // start timer - 2nd arg
    {
        const Command cmd1{CommandID::START_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        const Command cmd2{CommandID::START_TIMER, priority, as_number(RelayID::ONE), 2u, 4u};
        QTest::newRow("start timer - 2nd arg") << cmd1 << cmd2;
    }

    // set timer
    {
        const Command cmd1{CommandID::SET_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        const Command cmd2{CommandID::START_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        QTest::newRow("set timer") << cmd1 << cmd2;
    }

    // set timer - 1st arg
    {
        const Command cmd1{CommandID::SET_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        const Command cmd2{CommandID::SET_TIMER, priority, as_number(RelayID::ONE), 3u, 3u};
        QTest::newRow("set timer - 1st arg") << cmd1 << cmd2;
    }

    // set timer - 2nd arg
    {
        const Command cmd1{CommandID::SET_TIMER, priority, as_number(RelayID::ONE), 2u, 3u};
        const Command cmd2{CommandID::SET_TIMER, priority, as_number(RelayID::ONE), 2u, 4u};
        QTest::newRow("set timer - 2nd arg") << cmd1 << cmd2;
    }

    // query timer
    {
        const Command cmd1{CommandID::TIMER, priority, as_number(RelayID::ONE), as_number(TimerDelayType::TOTAL), 3u};
        const Command cmd2{CommandID::SET_TIMER, priority, as_number(RelayID::ONE), as_number(TimerDelayType::TOTAL),
            3u};
        QTest::newRow("query timer") << cmd1 << cmd2;
    }

    // query timer - 1st arg
    {
        const Command cmd1{CommandID::TIMER, priority, as_number(RelayID::ONE), as_number(TimerDelayType::TOTAL), 3u};
        const Command cmd2{CommandID::TIMER, priority, as_number(RelayID::ONE), as_number(TimerDelayType::REMAINING),
            3u};
        QTest::newRow("query timer - 1st arg") << cmd1 << cmd2;
    }

    // toggle relay
    {
        const Command cmd1{CommandID::TOGGLE_RELAY, priority, as_number(RelayID::ONE), 2u, 3u};
        const Command cmd2{CommandID::RELAY_OFF, priority, as_number(RelayID::ONE), 2u, 3u};
        QTest::newRow("toggle relay") << cmd1 << cmd2;
    }

    // query relay status
    {
        const Command cmd1{CommandID::QUERY_RELAY, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::BUTTON_MODE, priority, 1u, 2u, 3u};
        QTest::newRow("query relay status") << cmd1 << cmd2;
    }

    // set button mode
    {
        const Command cmd1{CommandID::SET_BUTTON_MODE, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::BUTTON_MODE, priority, 1u, 2u, 3u};
        QTest::newRow("set button mode") << cmd1 << cmd2;
    }

    // query button mode
    {
        const Command cmd1{CommandID::BUTTON_MODE, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::SET_BUTTON_MODE, priority, 1u, 2u, 3u};
        QTest::newRow("query button mode") << cmd1 << cmd2;
    }

    // reset factory defaults
    {
        const Command cmd1{CommandID::RESET_FACTORY_DEFAULTS, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::JUMPER_STATUS, priority, 1u, 2u, 3u};
        QTest::newRow("reset factory defaults") << cmd1 << cmd2;
    }

    // jumper status
    {
        const Command cmd1{CommandID::JUMPER_STATUS, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::FIRMWARE_VERSION, priority, 1u, 2u, 3u};
        QTest::newRow("jumper status") << cmd1 << cmd2;
    }

    // firmware version
    {
        const Command cmd1{CommandID::FIRMWARE_VERSION, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::JUMPER_STATUS, priority, 1u, 2u, 3u};
        QTest::newRow("firmware version") << cmd1 << cmd2;
    }

    // none command
    {
        const Command cmd1{CommandID::NONE, priority, 1u, 2u, 3u};
        const Command cmd2{CommandID::TOGGLE_RELAY, priority, 1u, 2u, 3u};
        QTest::newRow("none command") << cmd1 << cmd2;
    }
}


void CommandTest::isNotCompatible()
{
    // fetch data
    QFETCH(Command, command1);
    QFETCH(Command, command2);

    // test
    QVERIFY(!command1.isCompatible(command2));
}


void CardMessageTest::constructors()
{
    const unsigned char stx = 0x04;  // STX byte
    const unsigned char cmd = 0x21;  // set button mode
    const unsigned char mask = 0x10;  // momentary = relay 5
    const unsigned char param1 = 0xcf;  // toggle = all the other
    const unsigned char param2 = 0x20;  // timed = relay 6
    const unsigned char chk = 0xdc;  // check sum
    const unsigned char etx = 0x0f;  // ETX byte
    const unsigned char expected[7] = {stx, cmd, mask, param1, param2, chk, etx};

    // test constructor from data
    {
        const CardMessage message{stx, cmd, mask, param1, param2, chk, etx};
        for (int i = 0; i < 7; ++i) {
            QVERIFY2(message.data[i] == expected[i],
                qPrintable(QString{"data[%1] = '%2' does not match the expected %3."}
                    .arg(i).arg(message.data[i], 8, 2, QChar('0')).arg(expected[i], 8, 2, QChar('0'))));
        }
    }

    // test constructor from QByteArray
    {
        QByteArray byte_array = QByteArray::fromRawData(reinterpret_cast<const char *>(expected), 7);
        const CardMessage message{byte_array.constBegin(), byte_array.constEnd()};
        for (int i = 0; i < 7; ++i) {
            QVERIFY2(message.data[i] == expected[i],
                qPrintable(QString{"data[%1] = '%2' does not match the expected %3."}
                    .arg(i).arg(message.data[i], 8, 2, QChar('0')).arg(expected[i], 8, 2, QChar('0'))));
        }
    }


    // test constructor from raw C array
    {
        const CardMessage message{expected, &expected[7]};
        for (int i = 0; i < 7; ++i) {
            QVERIFY2(message.data[i] == expected[i],
                qPrintable(QString{"data[%1] = '%2' does not match the expected %3."}
                    .arg(i).arg(message.data[i], 8, 2, QChar('0')).arg(expected[i], 8, 2, QChar('0'))));
        }
    }
}


void CardMessageTest::checksumMessage()
{
    const unsigned char stx = 0x04;  // STX byte
    const unsigned char cmd = 0x21;  // set button mode
    const unsigned char mask = 0x10;  // momentary = relay 5
    const unsigned char param1 = 0xcf;  // toggle = all the other
    const unsigned char param2 = 0x20;  // timed = relay 6
    const unsigned char chk = 0xdc;  // check sum
    const unsigned char etx = 0x0f;  // ETX byte
    const unsigned char expected[7] = {stx, cmd, mask, param1, param2, chk, etx};

    CardMessage message{stx, cmd, mask, param1, param2, 0, etx};
    message.checksumMessage();
    for (int i = 0; i < 7; ++i) {
        QVERIFY2(message.data[i] == expected[i],
            qPrintable(QString{"data[%1] = '%2' does not match the expected %3."}
                .arg(i).arg(message.data[i], 8, 2, QChar('0')).arg(expected[i], 8, 2, QChar('0'))));
    }
}


void CardMessageTest::isValid()
{
    const unsigned char stx = 0x04;  // STX byte
    const unsigned char cmd = 0x21;  // set button mode
    const unsigned char mask = 0x10;  // momentary = relay 5
    const unsigned char param1 = 0xcf;  // toggle = all the other
    const unsigned char param2 = 0x20;  // timed = relay 6
    const unsigned char chk = 0xdc;  // check sum
    const unsigned char etx = 0x0f;  // ETX byte

    // valid message
    {
        CardMessage message{stx, cmd, mask, param1, param2, chk, etx};
        QVERIFY(message.isValid());
    }

    // bad stx
    {
        CardMessage message{stx - 1, cmd, mask, param1, param2, chk, etx};
        QVERIFY(!message.isValid());
    }

    // bad checksum
    {
        CardMessage message{stx, cmd, mask, param1, param2, chk - 1, etx};
        QVERIFY(!message.isValid());
    }

    // bad etx
    {
        CardMessage message{stx, cmd, mask, param1, param2, chk, etx - 1};
        QVERIFY(!message.isValid());
    }
}


void CardMessageTest::commandByte()
{
    const unsigned char stx = 0x04;  // STX byte
    const unsigned char cmd = 0x21;  // set button mode
    const unsigned char mask = 0x10;  // momentary = relay 5
    const unsigned char param1 = 0xcf;  // toggle = all the other
    const unsigned char param2 = 0x20;  // timed = relay 6
    const unsigned char chk = 0xdc;  // check sum
    const unsigned char etx = 0x0f;  // ETX byte

    CardMessage message{stx, cmd, mask, param1, param2, chk - 1, etx};
    QVERIFY2(message.commandByte() == cmd,
        qPrintable(QString{"commandByte = '%1' does not match the expected %2."}
            .arg(message.commandByte(), 8, 2, QChar('0')).arg(cmd, 8, 2, QChar('0'))));
}

}  // namespace impl_
}  // namespace k8090
}  // namespace core
}  // namespace sprelay
