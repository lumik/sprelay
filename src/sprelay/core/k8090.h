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
#include <type_traits>

#include "enum_flags.h"
#include "serial_port_utils.h"

// forward declarations
class QTimer;

namespace sprelay {
namespace core {

// forward declarations
class UnifiedSerialPort;

namespace command_queue {
// command_queue forward declaration
template<typename TCommand, int tSize>
class CommandQueue;
}  // namespace command_queue


namespace K8090Traits {
enum class CommandID : unsigned int
{
    RELAY_ON,
    RELAY_OFF,
    TOGGLE_RELAY,
    QUERY_RELAY,
    SET_BUTTON_MODE,
    BUTTON_MODE,
    START_TIMER,
    SET_TIMER,
    TIMER,
    RESET_FACTORY_DEFAULTS,
    JUMPER_STATUS,
    FIRMWARE_VERSION,
    NONE
};


enum class ResponseID : unsigned int
{
    BUTTON_MODE,       /*!< Response with button mode. */
    TIMER,             /*!< Response with timer delay. */
    BUTTON_STATUS,
    RELAY_STATUS,
    JUMPER_STATUS,     /*!< Response with jumper status. */
    FIRMWARE_VERSION,  /*!< Response with firmware version. */
    NONE               /*!< The number of all responses represents also none response. */
};


enum struct RelayID : unsigned char
{
    NONE  = 0,  /*!< None relay */
    ONE   = 1 << 0,
    TWO   = 1 << 1,
    THREE = 1 << 2,
    FOUR  = 1 << 3,
    FIVE  = 1 << 4,
    SIX   = 1 << 5,
    SEVEN = 1 << 6,
    EIGHT = 1 << 7,
    ALL   = 0xff     /*!< All relays. */
};


// this redefinition enables bitwise operator usage
constexpr bool enable_bitmask_operators(RelayID) { return true; }


constexpr RelayID from_number(unsigned int number) { return static_cast<RelayID>(1 << (number)); }


enum struct TimerDelayType : unsigned char
{
    TOTAL     = 0,
    REMAINING = 1 << 0,
    ALL       = 0xff
};


// conversion of scoped enum to underlying_type
template<typename E>
constexpr typename std::enable_if<std::is_enum<E>::value, std::underlying_type<E>>::type::type as_number(const E e)
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}


struct Command
{
    using IdType = CommandID;
    using NumberType = typename std::underlying_type<IdType>::type;

    Command() : id(CommandID::NONE), priority{0} {}
    explicit Command(IdType id, int priority = 0, unsigned char mask = 0, unsigned char param1 = 0,
            unsigned char param2 = 0)
        : id(id), priority{priority}, params{mask, param1, param2} {}
    static NumberType idAsNumber(IdType id) { return as_number(id); }

    IdType id;
    int priority;
    unsigned char params[3];

    Command & operator|=(const Command &other);

    bool operator==(const Command &other) const
    {
        if (id != other.id) {
            return false;
        }
        for (int i = 0; i < 3; ++i) {
            if (params[i] != other.params[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Command &other) const { return !(*this == other); }

    bool isCompatible(const Command &other) const;
};


}  // namespace K8090Traits


class K8090 : public QObject
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
    int pendingCommandCount(K8090Traits::CommandID id);

signals:  // NOLINT(whitespace/indent)
    void relayStatus(sprelay::core::K8090Traits::RelayID previous, sprelay::core::K8090Traits::RelayID current,
        sprelay::core::K8090Traits::RelayID timed);
    void buttonStatus(sprelay::core::K8090Traits::RelayID state, sprelay::core::K8090Traits::RelayID pressed,
        sprelay::core::K8090Traits::RelayID released);
    void totalTimerDelay(sprelay::core::K8090Traits::RelayID relay, quint16 delay);
    void remainingTimerDelay(sprelay::core::K8090Traits::RelayID relay, quint16 delay);
    void buttonModes(sprelay::core::K8090Traits::RelayID momentary, sprelay::core::K8090Traits::RelayID toggle,
       sprelay::core::K8090Traits::RelayID timed);
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
    void switchRelayOn(sprelay::core::K8090Traits::RelayID relays);
    void switchRelayOff(sprelay::core::K8090Traits::RelayID relays);
    void toggleRelay(sprelay::core::K8090Traits::RelayID relays);
    void setButtonMode(sprelay::core::K8090Traits::RelayID momentary, sprelay::core::K8090Traits::RelayID toggle,
        sprelay::core::K8090Traits::RelayID timed);
    void startRelayTimer(sprelay::core::K8090Traits::RelayID relays, quint16 delay = 0);
    void setRelayTimerDelay(sprelay::core::K8090Traits::RelayID relays, quint16 delay);
    void queryRelayStatus();
    void queryTotalTimerDelay(sprelay::core::K8090Traits::RelayID relays);
    void queryRemainingTimerDelay(sprelay::core::K8090Traits::RelayID relays);
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
    void sendCommand(K8090Traits::CommandID command_id, K8090Traits::RelayID mask = K8090Traits::RelayID::NONE,
            unsigned char param1 = 0, unsigned char param2 = 0);
    void enqueueCommand(K8090Traits::CommandID command_id, K8090Traits::RelayID mask = K8090Traits::RelayID::NONE,
                        unsigned char param1 = 0, unsigned char param2 = 0);
    bool updateCommand(
            const K8090Traits::Command &command,
            const QList<const K8090Traits::Command *> &pending_command_list);
    void sendCommandHelper(K8090Traits::CommandID command_id, K8090Traits::RelayID mask = K8090Traits::RelayID::NONE,
            unsigned char param1 = 0, unsigned char param2 = 0);
    bool hasResponse(K8090Traits::CommandID command_id);
    void sendToSerial(std::unique_ptr<unsigned char []> buffer, int n);

    void buttonModeResponse(const unsigned char *buffer);
    void timerResponse(const unsigned char *buffer);
    void buttonStatusResponse(const unsigned char *buffer);
    void relayStatusResponse(const unsigned char *buffer);
    void jumperStatusResponse(const unsigned char *buffer);
    void firmwareVersionResponse(const unsigned char *buffer);

    static unsigned char checkSum(const unsigned char *msg, int n);
    static bool validateResponse(const unsigned char *msg);
    static inline unsigned char lowByte(quint16 delay) { return (delay)&(0xFF); }
    static inline unsigned char highByte(quint16 delay) { return (delay>>8)&(0xFF); }

    static const int kDefaultCommandDelay_;
    static const int kDefaultFailureDelay_;
    static const int kDefaultMaxFailureCount_;


    QString com_port_name_;
    std::unique_ptr<UnifiedSerialPort> serial_port_;

    std::unique_ptr<command_queue::CommandQueue<K8090Traits::Command,
                        K8090Traits::as_number(K8090Traits::CommandID::NONE)>>
        pending_commands_;
    K8090Traits::Command current_command_;
    std::unique_ptr<QTimer> command_timer_;
    std::unique_ptr<QTimer> failure_timer_;
    int failure_counter_;
    bool connected_;
    int command_delay_;
    int factory_defaults_command_delay_;
    int failure_delay_;
    int failure_max_count_;
};

}  // namespace core
}  // namespace sprelay

Q_DECLARE_METATYPE(sprelay::core::K8090Traits::RelayID)

#endif  // SPRELAY_CORE_K8090_H_
