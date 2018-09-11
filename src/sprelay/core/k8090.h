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

#ifndef SPRELAY_CORE_K8090_H_
#define SPRELAY_CORE_K8090_H_

#include <QList>
#include <QObject>

#include <memory>
#include <queue>

#include "k8090_defines.h"
#include "serial_port_defines.h"

#include "sprelay/sprelay_global.h"

// forward declarations
class QTimer;

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


class SPRELAY_EXPORT K8090 : public QObject
{
    Q_OBJECT

public:  // NOLINT(whitespace/indent)
    static const quint16 kProductID;
    static const quint16 kVendorID;

    explicit K8090(QObject *parent = nullptr);
    ~K8090() override;

    static QList<serial_utils::ComPortParams> availablePorts();
    void setComPortName(const QString &name);
    void setCommandDelay(int msec);
    void setFailureDelay(int msec);
    void setMaxFailureCount(int count);
    bool isConnected();
    int pendingCommandCount(k8090::CommandID id);

signals:  // NOLINT(whitespace/indent)
    void relayStatus(sprelay::core::k8090::RelayID previous, sprelay::core::k8090::RelayID current,
        sprelay::core::k8090::RelayID timed);
    void buttonStatus(sprelay::core::k8090::RelayID state, sprelay::core::k8090::RelayID pressed,
        sprelay::core::k8090::RelayID released);
    void totalTimerDelay(sprelay::core::k8090::RelayID relay, quint16 delay);
    void remainingTimerDelay(sprelay::core::k8090::RelayID relay, quint16 delay);
    void buttonModes(sprelay::core::k8090::RelayID momentary, sprelay::core::k8090::RelayID toggle,
       sprelay::core::k8090::RelayID timed);
    void jumperStatus(bool on);
    void firmwareVersion(int year, int week);
    void connected();
    void connectionFailed();
    void notConnected();
    void disconnected();

public slots:  // NOLINT(whitespace/indent)
    void connectK8090();
    void disconnect();
    void refreshRelaysInfo();
    void switchRelayOn(sprelay::core::k8090::RelayID relays);
    void switchRelayOff(sprelay::core::k8090::RelayID relays);
    void toggleRelay(sprelay::core::k8090::RelayID relays);
    void setButtonMode(sprelay::core::k8090::RelayID momentary, sprelay::core::k8090::RelayID toggle,
        sprelay::core::k8090::RelayID timed);
    // TODO(lumik): add method to toggle button mode of specified buttons which treats buttons with no mode set as
    // buttons, the mode of which would not be modified.
    void startRelayTimer(sprelay::core::k8090::RelayID relays, quint16 delay = 0);
    void setRelayTimerDelay(sprelay::core::k8090::RelayID relays, quint16 delay);
    void queryRelayStatus();
    void queryTotalTimerDelay(sprelay::core::k8090::RelayID relays);
    void queryRemainingTimerDelay(sprelay::core::k8090::RelayID relays);
    void queryButtonModes();
    void resetFactoryDefaults();
    void queryJumperStatus();
    void queryFirmwareVersion();
    // TODO(lumik): use undocumented feature which enables to disable physical button by setting all its button modes
    // to zero. See the UnifiedSerialPort tests.

private slots:  // NOLINT(whitespace/indent)
    void onReadyData();
    void dequeueCommand();
    void onCommandFailed();

private:  // NOLINT(whitespace/indent)
    void sendCommand(k8090::CommandID command_id, k8090::RelayID mask = k8090::RelayID::None,
            unsigned char param1 = 0, unsigned char param2 = 0);
    void enqueueCommand(k8090::CommandID command_id, k8090::RelayID mask = k8090::RelayID::None,
                        unsigned char param1 = 0, unsigned char param2 = 0);
    void sendCommandHelper(k8090::CommandID command_id, k8090::RelayID mask = k8090::RelayID::None,
            unsigned char param1 = 0, unsigned char param2 = 0);
    bool hasResponse(k8090::CommandID command_id);
    void sendToSerial(std::unique_ptr<unsigned char []> buffer, int n);

    void buttonModeResponse(std::unique_ptr<impl_::CardMessage> response);
    void timerResponse(std::unique_ptr<impl_::CardMessage> response);
    void buttonStatusResponse(std::unique_ptr<impl_::CardMessage> response);
    void relayStatusResponse(std::unique_ptr<impl_::CardMessage> response);
    void jumperStatusResponse(std::unique_ptr<impl_::CardMessage> response);
    void firmwareVersionResponse(std::unique_ptr<impl_::CardMessage> response);
    void connectionSuccessful();
    void doDisconnect();

    static inline unsigned char lowByte(quint16 delay) { return (delay)&(0xFF); }
    static inline unsigned char highByte(quint16 delay) { return (delay>>8)&(0xFF); }

    static const int kDefaultCommandDelay_;
    static const int kDefaultFailureDelay_;
    static const int kDefaultMaxFailureCount_;


    QString com_port_name_;
    std::unique_ptr<UnifiedSerialPort> serial_port_;

    std::unique_ptr<impl_::ConcurentCommandQueue>
        pending_commands_;
    std::unique_ptr<k8090::impl_::Command> current_command_;
    std::unique_ptr<QTimer> command_timer_;
    std::unique_ptr<QTimer> failure_timer_;
    int failure_counter_;
    bool connected_;
    bool connecting_;
    int command_delay_;
    int factory_defaults_command_delay_;
    int failure_delay_;
    int failure_max_count_;
};

}  // namespace k8090
}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_K8090_H_
