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
 * \file      k8090.h
 * \ingroup   group_biomolecules_sprelay_core_public
 * \brief     The biomolecules::sprelay::core::k8090::K8090 class which provides API to control the relay card.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2017-03-22
 * \copyright Copyright (C) 2017 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */

#ifndef BIOMOLECULES_SPRELAY_CORE_K8090_H_
#define BIOMOLECULES_SPRELAY_CORE_K8090_H_

#include <memory>
#include <queue>

#include <QList>
#include <QObject>

#include "biomolecules/sprelay/sprelay_global.h"

#include "k8090_defines.h"
#include "serial_port_defines.h"

// forward declarations
class QMutex;
class QTimer;

namespace biomolecules {
namespace sprelay {
namespace core {

// forward declarations
class UnifiedSerialPort;

namespace k8090 {
namespace impl_ {
// Command forward declaration
struct Command;
// command_queue forward declaration
class ConcurentCommandQueue;
// CardMessage forward declaration
struct CardMessage;
}  // namespace impl_

/// The class that provides the interface for Velleman %K8090 relay card controlling through serial port.
class SPRELAY_LIBRARY_EXPORT K8090 : public QObject
{
    Q_OBJECT

public:
    static const quint16 kProductID;
    static const quint16 kVendorID;

    explicit K8090(QObject* parent = nullptr);
    ~K8090() override;

    static QList<serial_utils::ComPortParams> availablePorts();
    QString comPortName();
    void setComPortName(const QString& name);
    void setCommandDelay(int msec);
    void setFailureDelay(int msec);
    void setMaxFailureCount(int count);
    bool isConnected();
    int pendingCommandCount(k8090::CommandID id);

signals:
    void relayStatus(biomolecules::sprelay::core::k8090::RelayID previous,
        biomolecules::sprelay::core::k8090::RelayID current, biomolecules::sprelay::core::k8090::RelayID timed);
    void buttonStatus(biomolecules::sprelay::core::k8090::RelayID state,
        biomolecules::sprelay::core::k8090::RelayID pressed, biomolecules::sprelay::core::k8090::RelayID released);
    void totalTimerDelay(biomolecules::sprelay::core::k8090::RelayID relay, quint16 delay);
    void remainingTimerDelay(biomolecules::sprelay::core::k8090::RelayID relay, quint16 delay);
    void buttonModes(biomolecules::sprelay::core::k8090::RelayID momentary,
        biomolecules::sprelay::core::k8090::RelayID toggle, biomolecules::sprelay::core::k8090::RelayID timed);
    void jumperStatus(bool on);
    void firmwareVersion(int year, int week);
    void connected();
    void connectionFailed();
    void notConnected();
    void disconnected();
    void doDisconnect(bool failure);
    void enqueueCommand(biomolecules::sprelay::core::k8090::CommandID command_id);
    void enqueueCommand(biomolecules::sprelay::core::k8090::CommandID command_id,
        biomolecules::sprelay::core::k8090::RelayID mask);
    void enqueueCommand(biomolecules::sprelay::core::k8090::CommandID command_id,
        biomolecules::sprelay::core::k8090::RelayID mask, unsigned char param1);
    void enqueueCommand(biomolecules::sprelay::core::k8090::CommandID command_id,
        biomolecules::sprelay::core::k8090::RelayID mask, unsigned char param1, unsigned char param2);

public slots:
    void connectK8090();
    void disconnect();
    void refreshRelaysInfo();
    void switchRelayOn(biomolecules::sprelay::core::k8090::RelayID relays);
    void switchRelayOff(biomolecules::sprelay::core::k8090::RelayID relays);
    void toggleRelay(biomolecules::sprelay::core::k8090::RelayID relays);
    void setButtonMode(biomolecules::sprelay::core::k8090::RelayID momentary,
        biomolecules::sprelay::core::k8090::RelayID toggle, biomolecules::sprelay::core::k8090::RelayID timed);
    // TODO(lumik): add method to toggle button mode of specified buttons which treats buttons with no mode set as
    // buttons, the mode of which would not be modified.
    void startRelayTimer(biomolecules::sprelay::core::k8090::RelayID relays, quint16 delay = 0);
    void setRelayTimerDelay(biomolecules::sprelay::core::k8090::RelayID relays, quint16 delay);
    void queryRelayStatus();
    void queryTotalTimerDelay(biomolecules::sprelay::core::k8090::RelayID relays);
    void queryRemainingTimerDelay(biomolecules::sprelay::core::k8090::RelayID relays);
    void queryButtonModes();
    void resetFactoryDefaults();
    void queryJumperStatus();
    void queryFirmwareVersion();
    // TODO(lumik): use undocumented feature which enables to disable physical button by setting all its button modes
    // to zero. See the UnifiedSerialPort tests.

private slots:
    void onReadyData();
    void dequeueCommand();
    void onCommandFailed();
    void onDoDisconnect(bool failure);

private:
    void sendCommand(k8090::CommandID command_id, k8090::RelayID mask = k8090::RelayID::None, unsigned char param1 = 0,
        unsigned char param2 = 0);
    void onEnqueueCommand(k8090::CommandID command_id, k8090::RelayID mask = k8090::RelayID::None,
        unsigned char param1 = 0, unsigned char param2 = 0);
    void sendCommandHelper(k8090::CommandID command_id, k8090::RelayID mask = k8090::RelayID::None,
        unsigned char param1 = 0, unsigned char param2 = 0);
    bool hasResponse(k8090::CommandID command_id);
    void sendToSerial(std::unique_ptr<unsigned char[]> buffer, int n);

    void buttonModeResponse(std::unique_ptr<impl_::CardMessage> response);
    void timerResponse(std::unique_ptr<impl_::CardMessage> response);
    void buttonStatusResponse(std::unique_ptr<impl_::CardMessage> response);
    void relayStatusResponse(std::unique_ptr<impl_::CardMessage> response);
    void jumperStatusResponse(std::unique_ptr<impl_::CardMessage> response);
    void firmwareVersionResponse(std::unique_ptr<impl_::CardMessage> response);
    void connectionSuccessful();

    static inline unsigned char lowByte(quint16 delay) { return (delay) & (0xFF); }
    static inline unsigned char highByte(quint16 delay) { return (delay >> 8) & (0xFF); }

    static const int kDefaultCommandDelay_;
    static const int kDefaultFailureDelay_;
    static const int kDefaultMaxFailureCount_;


    QString com_port_name_;
    std::unique_ptr<QMutex> com_port_name_mutex_;
    std::unique_ptr<UnifiedSerialPort> serial_port_;

    std::unique_ptr<impl_::ConcurentCommandQueue> pending_commands_;
    std::unique_ptr<k8090::impl_::Command> current_command_;
    std::unique_ptr<QTimer> command_timer_;
    std::unique_ptr<QTimer> failure_timer_;
    int failure_counter_;
    bool connected_;
    bool connecting_;
    std::unique_ptr<QMutex> connected_mutex_;
    int command_delay_;
    int factory_defaults_command_delay_;
    std::unique_ptr<QMutex> command_delay_mutex_;
    int failure_delay_;
    std::unique_ptr<QMutex> failure_delay_mutex_;
    int failure_max_count_;
    std::unique_ptr<QMutex> failure_max_count_mutex_;
};

}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPRELAY_CORE_K8090_H_
