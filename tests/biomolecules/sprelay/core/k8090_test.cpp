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
 * \file      k8090_test.cpp
 * \brief     The biomolecules::sprelay::core::k8090::K8090Test class which implements tests for
 *            biomolecules::sprelay::core::k8090::K8090.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2017-03-21
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "k8090_test.h"

#include <QList>
#include <QSignalSpy>
#include <QtTest>
#include <QVariant>

#include "biomolecules/sprelay/core/k8090_commands.h"
#include "biomolecules/sprelay/core/serial_port_utils.h"
#include "biomolecules/sprelay/core/unified_serial_port.h"

// dirty trick which enables us to test private methods. Think of something
// else.
// #define private public
// #include "k8090.h"

namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {

void K8090Test::initTestCase()
{
    real_card_present_ = false;
    foreach (const serial_utils::ComPortParams& params,  // NOLINT(whitespace/parens)
            K8090::availablePorts()) {
        if (params.product_identifier == K8090::kProductID
                && params.vendor_identifier == K8090::kVendorID) {
            if (params.port_name != k8090::impl_::kMockPortName) {
                real_card_port_name_ = params.port_name;
                real_card_present_ = true;
                break;
            }
        }
    }

    // test if cards can be connected
    QList<QString> port_names;
    if (real_card_present_) {
        port_names << real_card_port_name_;
        qDebug() << "Real card port name:" << port_names.last();
    }
    port_names << k8090::impl_::kMockPortName;
    qDebug() << "Virtual card port name:" << port_names.last();

    for (auto port_name : port_names) {
        k8090_.reset(new K8090);
        k8090_->setComPortName(port_name);
        QSignalSpy spy_connect(k8090_.get(), SIGNAL(connected()));
        k8090_->connectK8090();
        if (spy_connect.count() < 1) {
            QVERIFY2(spy_connect.wait(), "Real card was not connected!");
        } else {
            QCOMPARE(spy_connect.count(), 1);
        }
    }
    k8090_.reset();
    // TODO(lumik): store initial relay state (button statuses, button modes, timer delays) at the beginning and
    // restore it at the end
}


void K8090Test::init()
{
    k8090_.reset(new K8090);
    QFETCH(QString, port_name);
    k8090_->setComPortName(port_name);
    QSignalSpy spy(k8090_.get(), SIGNAL(connected()));
    k8090_->connectK8090();
    if (spy.count() < 1) {
        QVERIFY2(spy.wait(), "Card was not connected!");
    }
    QCOMPARE(spy.count(), 1);
}


void K8090Test::cleanup()
{
    k8090_.reset();
}


void K8090Test::connectK8090_data()
{
    createTestData();
}


void K8090Test::connectK8090()
{
    QSignalSpy spy_connect(k8090_.get(), SIGNAL(connected()));
    QSignalSpy spy_relay_status(k8090_.get(),
        SIGNAL(relayStatus(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_button_modes(k8090_.get(),
        SIGNAL(buttonModes(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_total_timer_delay(k8090_.get(),
        SIGNAL(totalTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_remaining_timer_delay(k8090_.get(),
        SIGNAL(remainingTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_jumper_status(k8090_.get(), SIGNAL(jumperStatus(bool)));
    QSignalSpy spy_firmware_version(k8090_.get(), SIGNAL(firmwareVersion(int, int)));
    k8090_->connectK8090();
    if (spy_connect.count() < 1) {
        QVERIFY2(spy_connect.wait(), "Real card was not connected!");
    } else {
        QCOMPARE(spy_connect.count(), 1);
    }
    // compare, if connect method emited all the expected responsnes
    QCOMPARE(spy_relay_status.count(), 1);
    QCOMPARE(spy_button_modes.count(), 1);
    QCOMPARE(spy_total_timer_delay.count(), 8);
    QCOMPARE(spy_remaining_timer_delay.count(), 8);
    QCOMPARE(spy_jumper_status.count(), 1);
    QCOMPARE(spy_firmware_version.count(), 1);
}


void K8090Test::disconnect_data()
{
    createTestData();
}


void K8090Test::disconnect()
{
    QVERIFY2(k8090_->isConnected(), "Card was should be connected now!");
    QSignalSpy spy(k8090_.get(), SIGNAL(disconnected()));
    k8090_->disconnect();
    if (spy.count() < 1) {
        QVERIFY2(spy.wait(), "Card was not connected!");
    }
    QCOMPARE(spy.count(), 1);
    QVERIFY2(!k8090_->isConnected(), "Card was should be disconnected now!");
}


void K8090Test::refreshRelaysInfo_data()
{
    createTestData();
}


void K8090Test::refreshRelaysInfo()
{
    const int kTimerCount = 8;
    // connect signals
    QSignalSpy spy_relay_status(k8090_.get(),
        SIGNAL(relayStatus(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_button_modes(k8090_.get(),
        SIGNAL(buttonModes(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_total_timer_delay(k8090_.get(),
        SIGNAL(totalTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_remaining_timer_delay(k8090_.get(),
        SIGNAL(remainingTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_jumper_status(k8090_.get(), SIGNAL(jumperStatus(bool)));
    QSignalSpy spy_firmware_version(k8090_.get(), SIGNAL(firmwareVersion(int, int)));

    // send the command
    k8090_->refreshRelaysInfo();

    // test relay status signal
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    QList<QVariant> relay_status_arguments = spy_relay_status.takeFirst();
    qDebug() << QString("relay status: previous: %1, current: %2, timed: %3")
        .arg(qvariant_cast<unsigned char>(relay_status_arguments.at(0)), 8, 2, QChar('0'))
        .arg(qvariant_cast<unsigned char>(relay_status_arguments.at(1)), 8, 2, QChar('0'))
        .arg(qvariant_cast<unsigned char>(relay_status_arguments.at(2)), 8, 2, QChar('0'));

    // test button modes signal
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    QList<QVariant> button_modes_arguments = spy_button_modes.takeFirst();
    qDebug() << QString("button modes: momentary: %1, toggle: %2, timed: %3")
        .arg(qvariant_cast<unsigned char>(button_modes_arguments.at(0)), 8, 2, QChar('0'))
        .arg(qvariant_cast<unsigned char>(button_modes_arguments.at(1)), 8, 2, QChar('0'))
        .arg(qvariant_cast<unsigned char>(button_modes_arguments.at(2)), 8, 2, QChar('0'));

    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    QList<QVariant> total_timer_delay_arguments[kTimerCount];
    unsigned int number;
    unsigned int id;
    qDebug() << "Total timer delay:";
    for (int i = 0; i < kTimerCount; ++i) {
        total_timer_delay_arguments[i] = spy_total_timer_delay.takeFirst();
        id = qvariant_cast<unsigned char>(total_timer_delay_arguments[i].at(0));
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        qDebug() << QString("        relay %1: %2s")
            .arg(number)
            .arg(qvariant_cast<quint16>(total_timer_delay_arguments[i].at(1)));
    }

    // test remaining timer signals
    while (spy_remaining_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_remaining_timer_delay.wait(), "Remaining timer signal not received!");
    }
    QCOMPARE(spy_remaining_timer_delay.count(), 8);
    QList<QVariant> remaining_timer_delay_arguments[kTimerCount];
    qDebug() << "Remaining timer delay:";
    for (int i = 0; i < kTimerCount; ++i) {
        remaining_timer_delay_arguments[i] = spy_remaining_timer_delay.takeFirst();
        id = qvariant_cast<unsigned char>(remaining_timer_delay_arguments[i].at(0));
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        qDebug() << QString("        relay %1: %2s")
            .arg(number)
            .arg(qvariant_cast<quint16>(remaining_timer_delay_arguments[i].at(1)));
    }

    // test jumper status signal
    if (spy_jumper_status.count() < 1) {
        QVERIFY2(spy_jumper_status.wait(), "Jumper status signal not received!");
    }
    QCOMPARE(spy_jumper_status.count(), 1);
    QList<QVariant> jumper_status_arguments = spy_jumper_status.takeFirst();
    qDebug() << "jumper status:" << jumper_status_arguments.at(0).toBool();

    if (spy_firmware_version.count() < 1) {
        QVERIFY2(spy_firmware_version.wait(), "Firmware version signal not received!");
    }
    QCOMPARE(spy_firmware_version.count(), 1);
    QList<QVariant> firmware_version_arguments = spy_firmware_version.takeFirst();
    qDebug() << QString("Firmware version: year = %1, week = %2")
                .arg(firmware_version_arguments.at(0).toInt()).arg(firmware_version_arguments.at(1).toInt());
}


void K8090Test::switchRelayOnOff_data()
{
    createTestData();
}


void K8090Test::switchRelayOnOff()
{
    // TODO(lumik): test correct command merging
    const int kCommandDelay = 100;  // max delay between command and response

    // initial state
    bool initial_on = false;

    // connect signals
    QSignalSpy spy_relay_status(k8090_.get(),
        SIGNAL(relayStatus(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    // switch relay 1 off at first
    k8090_->switchRelayOff(RelayID::One);

    // the K8090 class should always respond. If nothing changes, the query relay status command is issued
    // automatically
    if (spy_relay_status.count() < 1) {
        spy_relay_status.wait(kCommandDelay);
    }
    QCOMPARE(spy_relay_status.count(), 1);
    // takeFirst probably decreases count
    QList<QVariant> relay_status_arguments = spy_relay_status.takeFirst();
    // test if the right arguments came
    // save the previous state of the relay 1
    RelayID previous = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    if (static_cast<bool>(previous & RelayID::One)) {
        qDebug() << "The relay 1 was initialy on.";
        initial_on = true;
    }
    // test if the relay 1 is realy currently off
    RelayID current = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QVERIFY2(static_cast<bool>(~current & RelayID::One), "The relay 1 should be now set off.");

    // switch relay 1 on
    k8090_->switchRelayOn(RelayID::One);

    // test relay status signal
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    // test if the right arguments came
    // test if relay 1 was previously off and now on
    previous = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    QVERIFY2(static_cast<bool>(~previous & RelayID::One), "The prvious status of relay 1 should be off.");
    current = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QVERIFY2(static_cast<bool>(current & RelayID::One), "The relay 1 should be now set on.");

    // if relay 1 was initially set off, set it off again
    if (!initial_on) {
        // switch relay 1 on
        k8090_->switchRelayOff(RelayID::One);

        // test relay status signal
        if (spy_relay_status.count() < 1) {
            QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
        }
        QCOMPARE(spy_relay_status.count(), 1);
        relay_status_arguments = spy_relay_status.takeFirst();
        // test if the right arguments came
        // test if relay 1 was previously on and now off
        previous = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
        QVERIFY2(static_cast<bool>(previous & RelayID::One),
            "The prvious status of relay 1 should be on.");
        current = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
        QVERIFY2(static_cast<bool>(~current & RelayID::One), "The relay 1 should be now set off.");
    }
}


void K8090Test::toggleRelay_data()
{
    createTestData();
}


void K8090Test::toggleRelay()
{
    const int kCommandDelay = 100;  // max delay between command and response

    // connect signals
    QSignalSpy spy_relay_status(k8090_.get(),
        SIGNAL(relayStatus(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    // toggle relay 2
    k8090_->toggleRelay(RelayID::Two);

    // test for response
    if (spy_relay_status.count() < 1) {
        spy_relay_status.wait(kCommandDelay);
    }
    QCOMPARE(spy_relay_status.count(), 1);

    QList<QVariant> relay_status_arguments = spy_relay_status.takeFirst();
    // test if the right arguments came
    // save the previous state of the relay 1
    RelayID previous = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    // test if the relay 1 is realy currently off
    RelayID current = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QVERIFY2(static_cast<bool>((current ^ previous) & RelayID::Two),
        "The status of relay 1 should change.");

    // toggle relay back
    k8090_->toggleRelay(RelayID::Two);

    // test relay status signal
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    // test if the relay is back to the same state
    current = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QVERIFY2(previous == current, "The relay 2 should be in the same state as before the test.");
}


void K8090Test::buttonMode_data()
{
    createTestData();
}


void K8090Test::buttonMode()
{
    // connect signals
    QSignalSpy spy_button_modes(k8090_.get(),
        SIGNAL(buttonModes(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));

    // get current button modes
    RelayID previous_momentary = RelayID::None;
    RelayID previous_toggle = RelayID::None;
    RelayID previous_timed = RelayID::None;
    k8090_->queryButtonModes();
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    QList<QVariant> button_modes_arguments = spy_button_modes.takeFirst();
    previous_momentary = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0));
    previous_toggle = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1));
    previous_timed = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2));
    qDebug() << QString("Previous modes: momentary: %1, toggle: %2, timed: %3")
        .arg(as_number(previous_momentary), 8, 2, QChar('0')).arg(as_number(previous_toggle), 8, 2, QChar('0'))
        .arg(as_number(previous_timed), 8, 2, QChar('0'));

    // create correct initial state
    RelayID momentary = RelayID::None;
    RelayID toggle = RelayID::All;
    RelayID timed = RelayID::None;
    if (previous_momentary != momentary || previous_toggle != toggle || previous_timed != timed) {
        k8090_->setButtonMode(momentary, toggle, timed);
        // test for response
        if (spy_button_modes.count() < 1) {
            QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
        }
        QCOMPARE(spy_button_modes.count(), 1);
        button_modes_arguments = spy_button_modes.takeFirst();
        QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0))
                 == RelayID::None,
                 "All relays should be in toggle mode!");
        QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1))
                 == RelayID::All,
                 "All relays should be in toggle mode!");
        QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2))
                 == RelayID::None,
                 "All relays should be in toggle mode!");
    }

    // set relay 1 to momentary mode
    momentary = RelayID::One;
    toggle = RelayID::All & ~RelayID::One;
    timed = RelayID::None;
    k8090_->setButtonMode(momentary, toggle, timed);
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    button_modes_arguments = spy_button_modes.takeFirst();
    QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0)) == momentary,
        "Relay one should be in momentary mode!");
    QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1)) == toggle,
        "All relays but relay one should be in toggle mode!");
    QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2)) == timed,
        "All relays should be in toggle or momentary mode!");

    // set relay 2 to timed mode
    momentary = RelayID::None;
    toggle = RelayID::All & ~RelayID::Two;
    timed = RelayID::Two;
    k8090_->setButtonMode(momentary, toggle, timed);
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    button_modes_arguments = spy_button_modes.takeFirst();
    QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0)) == momentary,
        "All relays should be in toggle or timed mode!");
    QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1)) == toggle,
        "All relays but relay two should be in toggle mode!");
    QVERIFY2(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2)) == timed,
        "Relay two should be in timed mode!");


    // reset initial settings
    k8090_->setButtonMode(previous_momentary, previous_toggle, previous_timed);
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    button_modes_arguments = spy_button_modes.takeFirst();
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0)),
        previous_momentary);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1)),
        previous_toggle);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2)), previous_timed);
}


void K8090Test::totalTimer_data()
{
    createTestData();
}


void K8090Test::totalTimer()
{
    const int kTimerCount = 8;

    // connect signals
    QSignalSpy spy_total_timer_delay(k8090_.get(),
        SIGNAL(totalTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));

    // get all total timers
    RelayID relay_ids = RelayID::All;
    k8090_->queryTotalTimerDelay(relay_ids);
    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    QList<QVariant> total_timer_delay_arguments[kTimerCount];
    quint16 initial_total_timers[kTimerCount];
    quint16 total_timers[kTimerCount];
    RelayID temp_id;
    unsigned int number;
    unsigned int id;
    qDebug() << "Total timer delay:";
    for (int i = 0; i < kTimerCount; ++i) {
        total_timer_delay_arguments[i] = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments[i].at(0));
        relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        initial_total_timers[number - 1] = qvariant_cast<quint16>(total_timer_delay_arguments[i].at(1));
        total_timers[number - 1] = initial_total_timers[number - 1];
        qDebug() << QString("        relay %1: %2s").arg(number).arg(initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);

    // set timers
    RelayID relay_id1 = RelayID::One;
    RelayID relay_ids2 = RelayID::Two | RelayID::Three;
    RelayID relay_ids3 = RelayID::Three | RelayID::Four;
    RelayID relay_ids4 = RelayID::Four | RelayID::Five;
    // set relay_id1 timer
    id = as_number(relay_id1);
    number = 0;
    while (id) {
        id >>= 1;
        ++number;
    }
    total_timers[number - 1] = initial_total_timers[number - 1] + 5;
    // first query relay so, all the following commands are enqueued while waiting for response
    k8090_->queryTotalTimerDelay(relay_id1);
    k8090_->setRelayTimerDelay(relay_id1, total_timers[number - 1]);
    QCOMPARE(k8090_->pendingCommandCount(CommandID::SetTimer), 1);

    // test if two consecutive set relay timer commands are merged
    unsigned int temp_i = 0;
    quint16 before_overlap_total_timers = total_timers[0];
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_ids2)) {
            total_timers[i] = initial_total_timers[i] + 10;
            temp_i = i;
            before_overlap_total_timers = total_timers[i];
        }
    }
    k8090_->setRelayTimerDelay(relay_ids2, total_timers[temp_i]);
    k8090_->setRelayTimerDelay(relay_ids3, total_timers[temp_i]);
    QCOMPARE(k8090_->pendingCommandCount(CommandID::SetTimer), 2);

    // test if set command with different number is not merged
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_ids4)) {
            total_timers[i] = initial_total_timers[i] + 15;
            temp_i = i;
        }
    }
    k8090_->setRelayTimerDelay(relay_ids4, total_timers[temp_i]);
    QCOMPARE(k8090_->pendingCommandCount(CommandID::SetTimer), 3);

    // test if query relay timer priority works compared to set relay timer
    relay_ids = relay_id1 | relay_ids2 | relay_ids3 | relay_ids4;
    int merged_timer_count = 0;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_ids)) {
            ++merged_timer_count;
        }
    }

    // test if two queries are merged together and the query timer has bigger priority than set timer
    k8090_->queryTotalTimerDelay(relay_ids2);
    k8090_->queryTotalTimerDelay(relay_ids3);
    k8090_->queryTotalTimerDelay(relay_ids4);
    QCOMPARE(k8090_->pendingCommandCount(CommandID::Timer), 1);

    // test total timer delay signals as response for query relay timer commands
    while (spy_total_timer_delay.count() < merged_timer_count) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), merged_timer_count);
    quint16 temp_total_timer;
    QList<QVariant> merged_total_timer_delay_arguments;
    for (int i = 0; i < merged_timer_count; ++i) {
        merged_total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(merged_total_timer_delay_arguments.at(0));
        relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(merged_total_timer_delay_arguments.at(1));
        QCOMPARE(temp_total_timer, initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);

    // test total timer delay signals as response for set relay timer commands
    relay_ids = relay_id1 | relay_ids2 | relay_ids3 | relay_ids4;
    RelayID overlap_ids = relay_ids3 & relay_ids4;
    // add responses to overlaping set timer delay commands
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & overlap_ids)) {
            ++merged_timer_count;
        }
    }
    while (spy_total_timer_delay.count() < merged_timer_count) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), merged_timer_count);
    for (int i = 0; i < merged_timer_count; ++i) {
        merged_total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(merged_total_timer_delay_arguments.at(0));
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(merged_total_timer_delay_arguments.at(1));
        if (static_cast<bool>(overlap_ids & temp_id)) {
            overlap_ids &= ~temp_id;
            QCOMPARE(temp_total_timer, before_overlap_total_timers);
        } else {
            relay_ids &= ~temp_id;
            QCOMPARE(temp_total_timer, total_timers[number - 1]);
        }
    }
    QCOMPARE(overlap_ids, RelayID::None);
    QCOMPARE(relay_ids, RelayID::None);

    // reset initial values
    relay_ids = RelayID::All;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        k8090_->setRelayTimerDelay(from_number(i), initial_total_timers[i]);
    }
    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    for (int i = 0; i < kTimerCount; ++i) {
        merged_total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(merged_total_timer_delay_arguments.at(0));
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(merged_total_timer_delay_arguments.at(1));
        relay_ids &= ~temp_id;
        QCOMPARE(temp_total_timer, initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);
}


void K8090Test::startTimer_data()
{
    createTestData();
}


void K8090Test::startTimer()
{
    const int kTimerCount = 8;
    const int kTimerWaitLimit = 1000;  // ms

    // connect signals
    QSignalSpy spy_total_timer_delay(k8090_.get(),
        SIGNAL(totalTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_remaining_timer_delay(k8090_.get(),
        SIGNAL(remainingTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_relay_status(k8090_.get(),
        SIGNAL(relayStatus(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));


    // get used initial timers delay
    quint16 timer_delay1 = 1;
    quint16 timer_delay2 = 2;
    RelayID relay_id1 = RelayID::One;
    RelayID relay_ids2 =  RelayID::Two | RelayID::Three;
    RelayID relay_ids = relay_id1 | relay_ids2;
    k8090_->queryTotalTimerDelay(relay_ids);

    // test total timer delay signals
    int timer_count = 0;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_ids)) {
            ++timer_count;
        }
    }
    while (spy_total_timer_delay.count() < timer_count) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), timer_count);
    QList<QVariant> total_timer_delay_arguments;
    quint16 initial_total_timers[kTimerCount];
    RelayID temp_id;
    unsigned int number;
    unsigned int id;
    for (int i = 0; i < timer_count; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        initial_total_timers[number - 1] = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
    }
    QCOMPARE(relay_ids, RelayID::None);

    // set total timer delays
    relay_ids = relay_id1 | relay_ids2;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_id1)) {
            k8090_->setRelayTimerDelay(from_number(i), timer_delay1);
        }
        if (static_cast<bool>(from_number(i) & relay_ids2)) {
            k8090_->setRelayTimerDelay(from_number(i), timer_delay2);
        }
    }
    // test total timer delay signals
    while (spy_total_timer_delay.count() < timer_count) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), timer_count);
    quint16 temp_total_timer;
    for (int i = 0; i < timer_count; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
        relay_ids &= ~temp_id;
        if (static_cast<bool>(from_number(i) & relay_id1)) {
            QCOMPARE(temp_total_timer, timer_delay1);
        }
        if (static_cast<bool>(from_number(i) & relay_ids2)) {
            QCOMPARE(temp_total_timer, timer_delay2);
        }
    }
    QCOMPARE(relay_ids, RelayID::None);

    // get status of affected relays
    k8090_->queryRelayStatus();
    // test relay status signal
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    RelayID initially_on =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));
    qDebug() << QString("These relays are on: %1")
        .arg(as_number(initially_on), 8, 2, QChar('0'));

    // switch affected relays off
    relay_ids = relay_id1 | relay_ids2;
    k8090_->switchRelayOff(relay_ids);
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    RelayID currently_on =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));
    for (int i = 0; i < 8; ++i) {
        if (static_cast<bool>(from_number(i) & relay_ids)) {
            QVERIFY2(static_cast<bool>(from_number(i) & ~currently_on),
                qPrintable(QString("Relay %1 is not off").arg(i + 1)));
        }
    }

    // start all selected timers without delay specified
    k8090_->startRelayTimer(relay_ids);
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    QList<QVariant> relay_status_arguments = spy_relay_status.takeFirst();
    RelayID previously_on =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QCOMPARE(relay_ids, currently_on & ~previously_on);

    // query remaining timer delays
    RelayID remaining_relay_ids = relay_id1 | relay_ids2;
    k8090_->queryRemainingTimerDelay(remaining_relay_ids);

    int remaining_timer_count = 0;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & remaining_relay_ids)) {
            ++remaining_timer_count;
        }
    }
    while (spy_remaining_timer_delay.count() < remaining_timer_count) {
        QVERIFY2(spy_remaining_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_remaining_timer_delay.count(), remaining_timer_count);
    QList<QVariant> remaining_timer_delay_arguments;
    for (int i = 0; i < remaining_timer_count; ++i) {
        remaining_timer_delay_arguments = spy_remaining_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(remaining_timer_delay_arguments.at(0));
        remaining_relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        qDebug() << QString("Relay %1: remaining delay = %2s")
                    .arg(number).arg(qvariant_cast<quint16>(remaining_timer_delay_arguments.at(1)));
    }
    QCOMPARE(remaining_relay_ids, RelayID::None);

    // wait for the first relay to timeout
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(timer_delay1 * 1000 + kTimerWaitLimit), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    previously_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QCOMPARE(relay_id1, previously_on & ~currently_on);

    // wait for the second relays to timeout
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait((timer_delay2 - timer_delay1) * 1000 + kTimerWaitLimit),
            "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    previously_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QCOMPARE(relay_ids2, previously_on & ~currently_on);

    // start all selected timers with specified delay, delay switched delay1 for timers2 and vice versa
    // first query timer delay, the next commands is enqueued then
    k8090_->queryRemainingTimerDelay(relay_id1);

    // start timers
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_id1)) {
            k8090_->startRelayTimer(from_number(i), timer_delay2);
        }
    }
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_ids2)) {
            k8090_->startRelayTimer(from_number(i), timer_delay1);
        }
    }
    // test, if the timer commands for relay_ids2 merged
    QCOMPARE(k8090_->pendingCommandCount(CommandID::StartTimer), 2);

    // pop out remaining timer query
    while (spy_remaining_timer_delay.count() < 1) {
        QVERIFY2(spy_remaining_timer_delay.wait(), "Remaining timer signal not received!");
    }
    QCOMPARE(spy_remaining_timer_delay.count(), 1);
    spy_remaining_timer_delay.takeFirst();

    // test relay 1 for start
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    previously_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QCOMPARE(relay_id1, ~previously_on & currently_on);

    // test relays 2 for start
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    previously_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QCOMPARE(relay_ids2, ~previously_on & currently_on);

    // query remaining timer delays
    remaining_relay_ids = relay_id1 | relay_ids2;
    k8090_->queryRemainingTimerDelay(remaining_relay_ids);

    remaining_timer_count = 0;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & remaining_relay_ids)) {
            ++remaining_timer_count;
        }
    }
    while (spy_remaining_timer_delay.count() < remaining_timer_count) {
        QVERIFY2(spy_remaining_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_remaining_timer_delay.count(), remaining_timer_count);
    for (int i = 0; i < remaining_timer_count; ++i) {
        remaining_timer_delay_arguments = spy_remaining_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(remaining_timer_delay_arguments.at(0));
        remaining_relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        qDebug() << QString("Relay %1: remaining delay = %2s")
                    .arg(number).arg(qvariant_cast<quint16>(remaining_timer_delay_arguments.at(1)));
    }
    QCOMPARE(remaining_relay_ids, RelayID::None);

    // wait for the first relay to timeout
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(timer_delay1 * 1000 + kTimerWaitLimit), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    previously_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QCOMPARE(relay_ids2, previously_on & ~currently_on);

    // wait for the second relays to timeout
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait((timer_delay2 - timer_delay1) * 1000 + kTimerWaitLimit),
            "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    relay_status_arguments = spy_relay_status.takeFirst();
    previously_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(0));
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(relay_status_arguments.at(1));
    QCOMPARE(relay_id1, previously_on & ~currently_on);

    // switch on relays which were initially on
    k8090_->switchRelayOn(initially_on);
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    currently_on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));
    QCOMPARE(initially_on, currently_on);

    // reset total timer delays at the end
    relay_ids = relay_id1 | relay_ids2;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        if (static_cast<bool>(from_number(i) & relay_ids)) {
            k8090_->setRelayTimerDelay(from_number(i), initial_total_timers[i]);
        }
    }
    // test total timer delay signals
    while (spy_total_timer_delay.count() < timer_count) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), timer_count);
    for (int i = 0; i < timer_count; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
        relay_ids &= ~temp_id;
        QCOMPARE(temp_total_timer, initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);
}


void K8090Test::factoryDefaults_data()
{
    createTestData();
}


void K8090Test::factoryDefaults()
{
    const int kTimerCount = 8;
    const RelayID kDefaultMomentary = RelayID::None;
    const RelayID kDefaultToggle = RelayID::All;
    const RelayID kDefaultTimed = RelayID::None;
    const quint16 kDefaultTimerDelay = 5;

    // connect signals
    QSignalSpy spy_relay_status(k8090_.get(),
        SIGNAL(relayStatus(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_button_modes(k8090_.get(),
        SIGNAL(buttonModes(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_total_timer_delay(k8090_.get(),
        SIGNAL(totalTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));

    // get current relay status
    k8090_->queryRelayStatus();
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    const RelayID initially_on =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));

    // get current button modes
    k8090_->queryButtonModes();
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    QList<QVariant> button_modes_arguments = spy_button_modes.takeFirst();
    const RelayID previous_momentary =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0));
    const RelayID previous_toggle =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1));
    const RelayID previous_timed =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2));
    qDebug() << QString("Previous modes: momentary: %1, toggle: %2, timed: %3")
        .arg(as_number(previous_momentary), 8, 2, QChar('0')).arg(as_number(previous_toggle), 8, 2, QChar('0'))
        .arg(as_number(previous_timed), 8, 2, QChar('0'));

    // get all total timers
    RelayID relay_ids = RelayID::All;
    k8090_->queryTotalTimerDelay(relay_ids);
    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    QList<QVariant> total_timer_delay_arguments;
    quint16 initial_total_timers[kTimerCount];
    RelayID temp_id;
    unsigned int number;
    unsigned int id;
    qDebug() << "Total timer delay:";
    for (int i = 0; i < kTimerCount; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        initial_total_timers[number - 1] = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
        qDebug() << QString("        relay %1: %2s").arg(number).arg(initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);

    // create initial state different from factory defaults
    RelayID momentary = RelayID::One;
    RelayID toggle = RelayID::All & ~RelayID::One & ~RelayID::Two;
    RelayID timed = RelayID::Two;
    quint16 timer1_delay = 10;
    k8090_->setButtonMode(momentary, toggle, timed);
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    button_modes_arguments = spy_button_modes.takeFirst();
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0)), momentary);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1)), toggle);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2)), timed);

    k8090_->setRelayTimerDelay(RelayID::One, timer1_delay);
    // test total timer delay signals
    if (spy_total_timer_delay.count() < 1) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), 1);
    total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
    temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
    QCOMPARE(temp_id, RelayID::One);
    quint16 temp_total_timer = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
    QCOMPARE(temp_total_timer, timer1_delay);

    // reset factory defaults
    k8090_->resetFactoryDefaults();
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    RelayID on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));
    QCOMPARE(on, RelayID::None);

    k8090_->queryButtonModes();
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    button_modes_arguments = spy_button_modes.takeFirst();
    momentary = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0));
    toggle = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1));
    timed = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2));
    QCOMPARE(momentary, kDefaultMomentary);
    QCOMPARE(toggle, kDefaultToggle);
    QCOMPARE(timed, kDefaultTimed);
    qDebug() << QString("Default modes: momentary: %1, toggle: %2, timed: %3")
        .arg(as_number(momentary), 8, 2, QChar('0')).arg(as_number(toggle), 8, 2, QChar('0'))
        .arg(as_number(timed), 8, 2, QChar('0'));

    relay_ids = RelayID::All;
    k8090_->queryTotalTimerDelay(relay_ids);
    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    qDebug() << "Default timer delay:";
    for (int i = 0; i < kTimerCount; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
        QCOMPARE(temp_total_timer, kDefaultTimerDelay);
        qDebug() << QString("        relay %1: %2s").arg(number).arg(temp_total_timer);
    }
    QCOMPARE(relay_ids, RelayID::None);

    // reset initial values
    k8090_->switchRelayOn(initially_on);
    k8090_->switchRelayOff(~initially_on);
    while (spy_relay_status.count() < 2) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();  // remove first status, which does not accont for switch relay off command
    on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));
    QCOMPARE(on, initially_on);

    k8090_->setButtonMode(previous_momentary, previous_toggle, previous_timed);
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    button_modes_arguments = spy_button_modes.takeFirst();
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0)),
        previous_momentary);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1)),
        previous_toggle);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2)), previous_timed);

    relay_ids = RelayID::All;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        k8090_->setRelayTimerDelay(from_number(i), initial_total_timers[i]);
    }
    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    for (int i = 0; i < kTimerCount; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
        relay_ids &= ~temp_id;
        QCOMPARE(temp_total_timer, initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);
}


void K8090Test::jumperStatus_data()
{
    createTestData();
}


void K8090Test::jumperStatus()
{
    QSignalSpy spy_jumper_status(k8090_.get(), SIGNAL(jumperStatus(bool)));
    k8090_->queryJumperStatus();
    if (spy_jumper_status.count() < 1) {
        QVERIFY2(spy_jumper_status.wait(), "Jumper status signal not received!");
    }
    QCOMPARE(spy_jumper_status.count(), 1);
    QList<QVariant> jumper_status_arguments = spy_jumper_status.takeFirst();
    qDebug() << "jumper status:" << jumper_status_arguments.at(0).toBool();
}


void K8090Test::firmwareVersion_data()
{
    createTestData();
}


void K8090Test::firmwareVersion()
{
    QSignalSpy spy_firmware_version(k8090_.get(), SIGNAL(firmwareVersion(int, int)));
    k8090_->queryFirmwareVersion();
    if (spy_firmware_version.count() < 1) {
        QVERIFY2(spy_firmware_version.wait(), "Firmware version signal not received!");
    }
    QCOMPARE(spy_firmware_version.count(), 1);
    QList<QVariant> firmware_version_arguments = spy_firmware_version.takeFirst();
    qDebug() << QString("Firmware version: year = %1, week = %2")
                .arg(firmware_version_arguments.at(0).toInt()).arg(firmware_version_arguments.at(1).toInt());
}


void K8090Test::priorities_data()
{
    createTestData();
}


void K8090Test::priorities()
{
    const int kTimerCount = 8;
    const int kTimerWaitLimit = 1000;  // ms
    const int kSpyNo = 6;

    // connect signals
    QSignalSpy spy_relay_status(k8090_.get(),
        SIGNAL(relayStatus(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_button_modes(k8090_.get(),
        SIGNAL(buttonModes(biomolecules::sprelay::core::k8090::RelayID, biomolecules::sprelay::core::k8090::RelayID,
            biomolecules::sprelay::core::k8090::RelayID)));
    QSignalSpy spy_total_timer_delay(k8090_.get(),
        SIGNAL(totalTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_remaining_timer_delay(k8090_.get(),
        SIGNAL(remainingTimerDelay(biomolecules::sprelay::core::k8090::RelayID, quint16)));
    QSignalSpy spy_jumper_status(k8090_.get(), SIGNAL(jumperStatus(bool)));
    QSignalSpy spy_firmware_version(k8090_.get(), SIGNAL(firmwareVersion(int, int)));
    QSignalSpy* spies[kSpyNo] = {&spy_relay_status, &spy_button_modes, &spy_total_timer_delay,
        &spy_remaining_timer_delay, &spy_jumper_status, &spy_firmware_version};

    // store initial state
    k8090_->queryRelayStatus();
    // test relay status signal
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    QCOMPARE(spy_relay_status.count(), 1);
    RelayID initially_on =
        qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));
    qDebug() << QString("These relays are on: %1")
        .arg(as_number(initially_on), 8, 2, QChar('0'));

    RelayID previous_momentary;
    RelayID previous_toggle;
    RelayID previous_timed;
    k8090_->queryButtonModes();
    // test for response
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    QList<QVariant> button_modes_arguments = spy_button_modes.takeFirst();
    previous_momentary = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0));
    previous_toggle = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1));
    previous_timed = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2));
    qDebug() << QString("Previous modes: momentary: %1, toggle: %2, timed: %3")
        .arg(as_number(previous_momentary), 8, 2, QChar('0')).arg(as_number(previous_toggle), 8, 2, QChar('0'))
        .arg(as_number(previous_timed), 8, 2, QChar('0'));

    // get all total timers
    RelayID relay_ids = RelayID::All;
    k8090_->queryTotalTimerDelay(relay_ids);
    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    QList<QVariant> total_timer_delay_arguments;
    quint16 initial_total_timers[kTimerCount];
    RelayID temp_id;
    unsigned int number;
    unsigned int id;
    qDebug() << "Total timer delay:";
    for (int i = 0; i < kTimerCount; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        relay_ids &= ~temp_id;
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        initial_total_timers[number - 1] = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
        qDebug() << QString("        relay %1: %2s").arg(number).arg(initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);

    // test priorities - queries has higher priority than set commands
    RelayID momentary = RelayID::One;
    RelayID toggle = RelayID::All & ~RelayID::One & ~RelayID::Two;
    RelayID timed = RelayID::Two;
    // first send any command, all the next commands will be added to the queue
    k8090_->resetFactoryDefaults();
    // set commands first
    k8090_->resetFactoryDefaults();
    k8090_->switchRelayOn(RelayID::Five);
    k8090_->switchRelayOff(RelayID::Six);
    k8090_->toggleRelay(RelayID::Seven);
    k8090_->setButtonMode(momentary, toggle, timed);
    k8090_->queryJumperStatus();
    k8090_->queryFirmwareVersion();
    k8090_->setRelayTimerDelay(RelayID::Three, 2);
    k8090_->startRelayTimer(RelayID::Eight, 1);
    // then query commands, which should move before set commands
    k8090_->queryRelayStatus();
    k8090_->queryTotalTimerDelay(RelayID::One);
    k8090_->queryRemainingTimerDelay(RelayID::One);
    k8090_->queryButtonModes();
    // TODO(lumik): test also if the output is correct
    // then control, if the answers comes in the right order
    // resetFactoryDefaults
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // queryRelayStatus
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // queryTotalTimerDelay
    if (spy_total_timer_delay.count() < 1) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer delay signal not received!");
    }
    spy_total_timer_delay.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // queryRemainingTimerDelay
    if (spy_remaining_timer_delay.count() < 1) {
        QVERIFY2(spy_remaining_timer_delay.wait(), "Remaining timer delay signal not received!");
    }
    spy_remaining_timer_delay.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // queryButtonModes
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    spy_button_modes.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // resetFactoryDefaults
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // switchRelayOn
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // switchRelayOff
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // toggleRelay
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // setButtonMode
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    spy_button_modes.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // queryJumperStatus
    if (spy_jumper_status.count() < 1) {
        QVERIFY2(spy_jumper_status.wait(), "Jumper status signal not received!");
    }
    spy_jumper_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // queryFirmwareVersion
    if (spy_firmware_version.count() < 1) {
        QVERIFY2(spy_firmware_version.wait(), "Firmware version signal not received!");
    }
    spy_firmware_version.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // setRelayTimerDelay
    if (spy_total_timer_delay.count() < 1) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer delay signal not received!");
    }
    spy_total_timer_delay.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // startRelayTimer
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");
    // timer elapsed
    if (spy_relay_status.count() < 1) {
        QVERIFY2(spy_relay_status.wait(1000 + kTimerWaitLimit), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();
    QVERIFY2(checkNoSpyData(spies, kSpyNo), "There shouldn't be any signal waiting now!");

    // reset initial values
    k8090_->switchRelayOn(initially_on);
    k8090_->switchRelayOff(~initially_on);
    while (spy_relay_status.count() < 2) {
        QVERIFY2(spy_relay_status.wait(), "Relay status signal not received!");
    }
    spy_relay_status.takeFirst();  // remove first status, which does not accont for switch relay off command
    RelayID on = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(spy_relay_status.takeFirst().at(1));
    QCOMPARE(on, initially_on);

    k8090_->setButtonMode(previous_momentary, previous_toggle, previous_timed);
    if (spy_button_modes.count() < 1) {
        QVERIFY2(spy_button_modes.wait(), "Button modes signal not received!");
    }
    QCOMPARE(spy_button_modes.count(), 1);
    button_modes_arguments = spy_button_modes.takeFirst();
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(0)),
        previous_momentary);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(1)),
        previous_toggle);
    QCOMPARE(qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(button_modes_arguments.at(2)), previous_timed);

    relay_ids = RelayID::All;
    for (unsigned int i = 0; i < kTimerCount; ++i) {
        k8090_->setRelayTimerDelay(from_number(i), initial_total_timers[i]);
    }
    // test total timer delay signals
    while (spy_total_timer_delay.count() < kTimerCount) {
        QVERIFY2(spy_total_timer_delay.wait(), "Total timer signal not received!");
    }
    QCOMPARE(spy_total_timer_delay.count(), kTimerCount);
    quint16 temp_total_timer;
    for (int i = 0; i < kTimerCount; ++i) {
        total_timer_delay_arguments = spy_total_timer_delay.takeFirst();
        temp_id = qvariant_cast<biomolecules::sprelay::core::k8090::RelayID>(total_timer_delay_arguments.at(0));
        id = as_number(temp_id);
        // calculate relay number from byte representation
        number = 0;
        while (id) {
            id >>= 1;
            ++number;
        }
        temp_total_timer = qvariant_cast<quint16>(total_timer_delay_arguments.at(1));
        relay_ids &= ~temp_id;
        QCOMPARE(temp_total_timer, initial_total_timers[number - 1]);
    }
    QCOMPARE(relay_ids, RelayID::None);
}


void K8090Test::createTestData()
{
    QTest::addColumn<QString>("port_name");

    if (real_card_present_) {
        QTest::newRow("real card") << real_card_port_name_;
    }
    QTest::newRow("virtual card") << k8090::impl_::kMockPortName;
}


bool K8090Test::checkNoSpyData(QSignalSpy** spies, int n)
{
    for (int i = 0; i < n; ++i) {
        if (spies[i]->count() > 0) {
            qDebug() << QString("Spy no. %1 has %2 signals waiting!").arg(i).arg(spies[i]->count());
            return false;
        }
    }
    return true;
}

}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
