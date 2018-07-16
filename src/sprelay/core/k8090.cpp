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
    \file k8090.cpp
*/

#include "k8090.h"

#include <QStringBuilder>
#include <QTimer>

#include <stdexcept>
#include <utility>

#include "command_queue.h"
#include "k8090_commands.h"
#include "k8090_utils.h"
#include "serial_port_utils.h"
#include "unified_serial_port.h"

/*!
    \brief Namespace containing sprelay application.
*/
namespace sprelay {

/*!
    \defgroup Core SpRelay core
    \brief Groups SpRelay core elements.
*/

/*!
    \ingroup Core
    \brief Namespace which contains SpRelay core elements.
*/
namespace core {

/*!
    \defgroup k8090 K8090 module
    \ingroup Core
    \brief K8090 class and related data structures.
*/

/*!
    \namespace sprelay::core::k8090
    \ingroup k8090
    \brief Contains K8090 class and related data structures.
*/
namespace k8090 {


/*!
    \class K8090
    \ingroup Core
    \brief The class that provides the interface for Velleman %K8090 relay card
    controlling through serial port.

    \remark reentrant, thread-safe
*/

// initialization of static member variables

// public
/*!
    \brief Product id for the automatic port identification.
    \sa K8090::connectK8090()
*/
const quint16 K8090::kProductID = impl_::kProductID;

/*!
    \brief Vendor id for the automatic port identification.
    \sa K8090::connectK8090()
*/
const quint16 K8090::kVendorID = impl_::kVendorID;

// private
// Shortest interval in ms from sending one command to sending a new one.
const int K8090::kDefaultCommandDelay_ = 50;
// Maximal time in ms to wait for response.
const int K8090::kDefaultFailureDelay_ = 1000;
// Maximal number of consecutive failures to disconnect realy;
const int K8090::kDefaultMaxFailureCount_ = 3;


/*!
  \brief Creates a new K8090 instance and sets the default values.
  \param parent K8090 parent object in Qt ownership system.
*/
K8090::K8090(QObject *parent) :
    QObject{parent},
    serial_port_{new UnifiedSerialPort},
    pending_commands_{new command_queue::CommandQueue<impl_::Command, as_number(CommandID::NONE)>},
    current_command_{new impl_::Command},
    command_timer_{new QTimer},
    failure_timer_{new QTimer},
    failure_counter_{0},
    connected_{false},
    connecting_{false},
    command_delay_{kDefaultCommandDelay_},
    factory_defaults_command_delay_{2 * kDefaultCommandDelay_},
    failure_delay_{kDefaultFailureDelay_},
    failure_max_count_{kDefaultMaxFailureCount_}
{
    command_timer_->setSingleShot(true);
    failure_timer_->setSingleShot(true);

    connect(serial_port_.get(), &UnifiedSerialPort::readyRead, this, &K8090::onReadyData);
    connect(command_timer_.get(), &QTimer::timeout, this, &K8090::dequeueCommand);
    connect(failure_timer_.get(), &QTimer::timeout, this, &K8090::onCommandFailed);
}


/*!
   \brief Destructor.
 */
K8090::~K8090()
{
    serial_port_->close();
}


/*!
  \brief Lists available serial ports.
  \return Available serial ports information list.
*/
QList<serial_utils::ComPortParams> K8090::availablePorts()
{
    return UnifiedSerialPort::availablePorts();
}


/*!
    \brief Sets new serial port name.

    If the new name is different from the previous one, it disconnects and closes serial port. If the object was
    connected, it also emits signal K8090::notConnected().
    \param name
*/
void K8090::setComPortName(const QString &name)
{
    if (com_port_name_ != name) {
        com_port_name_ = name;
        if (connected_ | connecting_) {
            doDisconnect();
            emit disconnected();
        }
    }
}

/*!
    \brief Sets command delay to msec.

    Command delay is shortest interval in ms from sending one command to sending a new one and is given by hardware
    limitations. If the commands are sended too close to each other, the virtual serial port communication merges them
    to one command which is then not recognized by the card.

    \param msec Desired command delay.
*/
void K8090::setCommandDelay(int msec)
{
    command_delay_ = msec;
    factory_defaults_command_delay_ = 2 * command_delay_;
}


/*!
    \brief Sets failure delay to msec.

    Failure delay is maximal time in ms to wait for a response. If the card doesn't response until failure delay
    elapses, the failure counter is increased. If the number of consecutive failures overflows max failure count (see
    K8090::setMaxFailureCount()), the K8090::connectionFailed() signal is emited.

    \param msec Desired command delay.
*/
void K8090::setFailureDelay(int msec)
{
    failure_delay_ = msec;
}

/*!
    \brief Sets max failure count.

    If the number of consecutive failures overflows max failure count, the K8090::connectionFailed() signal is emited.

    \param count The failure count.
    \sa K8090::setFailureDelay().
*/
void K8090::setMaxFailureCount(int count)
{
    failure_max_count_ = count;
}


/*!
    \brief Test if the relay is connected.
    \return True if connected.
*/
bool K8090::isConnected()
{
    return connected_;
}


/*!
    \brief Gets a number of commands waiting for execution in queue.
    \param id Queried command.
    \return The number of commands in queue.
*/
int K8090::pendingCommandCount(CommandID id)
{
    return pending_commands_->get(id).size();
}


// public signals
/*!
    \fn void K8090::relayStatus(k8090::RelayID previous, k8090::RelayID current,
            k8090::RelayID timed)
    \brief Emited when the Relay status event comes from the card.

    The Relay status event can come when the status of one or more ralays changes. The status can change because of
    button push or release or in the reaction to K8090::switchRelayOn(), K8090::switchRelayOff(), K8090::toggleRelay(),
    K8090::startTimer() or when the timer times out.

    You can get currently switched on or off relays for example by issuing these expressions:
    \code
    k8090::RelayID switched_on = (previous ^ current) & current;
    k8090::RelayID switched_off = (previous ^ current) & previous;
    \endcode

    See the Velleman %K8090 card manual for more details.

    \param previous Relays which were previously switched on.
    \param current Relays which are currently switched on.
    \param timed Timed relays.
*/
/*!
    \fn void K8090::buttonStatus(k8090::RelayID state, k8090::RelayID pressed,
            k8090::RelayID released)
    \brief Emited when the button is physically pressed or released.

    See the Velleman %K8090 card manual for more details.

    \param state Buttons which are pressed.
    \param pressed Buttons currently pressed.
    \param released Buttons currently released.
*/
/*!
    \fn void K8090::totalTimerDelay(k8090::RelayID relay, quint16 delay)
    \brief Reports total timer delay.

    This signal is emited as the reaction to the K8090::queryTotalTimerDelay(). If more relays is queried, the signal
    is emited for each relay separately. See the Velleman %K8090 card manual for more details.

    \param relay The relay.
    \param delay The delay.
*/
/*!
    \fn void K8090::remainingTimerDelay(k8090::RelayID relay, quint16 delay)
    \brief Reports current remaining timer delay.

    This signal is emited as the reaction to the K8090::queryRemainingTimerDelay(). If more relays is queried, the
    signal is emited for each relay separately. See the Velleman %K8090 card manual for more details.

    \param relay The relay.
    \param delay The delay.
*/
/*!
    \fn void K8090::buttonModes(k8090::RelayID momentary, k8090::RelayID toggle,
            k8090::RelayID timed)
    \brief Reports button modes.

    This signal is emited as the reaction to the K8090::queryButtonModes(). See the Velleman %K8090 card manual for
    more details.

    \param momentary Relays in momentary mode.
    \param toggle Relays in toggle mode.
    \param timed Relays in timed mode.
*/
/*!
    \fn void K8090::jumperStatus(bool on)
    \brief Reports jumper status.

    This signal is emited as the reaction to the K8090::queryJumperStatus(). If jumper is set on, pushing of the
    physical buttons no longer interacts with the relays but the button events are still emited. See the Velleman
    %K8090 card manual for more details.

    \param on True if the jumper is switched on.
*/
/*!
    \fn void K8090::firmwareVersion(int year, int week)
    \brief Reports firmware version.

    This signal is emited as the reaction to the K8090::queryFirmwareVersion(). It reports the year and week of
    firmware compilation. See the Velleman %K8090 card manual for more details.

    \param year The year.
    \param week The week.
*/
/*!
    \fn void K8090::connected()
    \brief Reports if the communication with the card was successfuly established.

    This signal is emited as the reaction to the K8090::connectK8090().
*/
/*!
    \fn void K8090::connectionFailed()
    \brief Reports communication failure after 3 consecutive unsuccessful trials.
*/
/*!
    \fn void K8090::notConnected()
    \brief Emited when you try to issue command on unconnected K8090 object.
*/
/*!
    \fn void K8090::disconnected()
    \brief This signal is emited as the reaction to the K8090::disconnect().
*/


// public slots
/*!
    \brief Connects to the relay card.

    It emits K8090::connected() signal if the connection is established. It also gets initial relay state, emiting
    K8090::relayStatus(), K8090::buttonModes(), K8090::totalTimerDelay(), K8090::remainingTimerDelay(),
    K8090::jumperStatus() and K8090::firmwareVersion() signals.
*/
void K8090::connectK8090()
{
    if (connecting_) {
        return;
    }
    connected_ = false;
    bool card_found = false;
    foreach (const serial_utils::ComPortParams &params,  // NOLINT(whitespace/parens)
            UnifiedSerialPort::availablePorts()) {
        if (params.port_name == com_port_name_
                && params.product_identifier == kProductID
                && params.vendor_identifier == kVendorID) {
            card_found = true;
        }
    }

    if (!card_found) {
        emit connectionFailed();
        return;
    }

    serial_port_->setPortName(com_port_name_);
    serial_port_->setBaudRate(QSerialPort::Baud19200);
    serial_port_->setDataBits(QSerialPort::Data8);
    serial_port_->setParity(QSerialPort::NoParity);
    serial_port_->setStopBits(QSerialPort::OneStop);
    serial_port_->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_port_->isOpen()) {
        if (!serial_port_->open(QIODevice::ReadWrite)) {
            emit connectionFailed();
            return;
        }
    }

    connecting_ = true;
    enqueueCommand(CommandID::QUERY_RELAY);
    enqueueCommand(CommandID::BUTTON_MODE);
    enqueueCommand(CommandID::TIMER, RelayID::ALL, as_number(impl_::TimerDelayType::TOTAL));
    enqueueCommand(CommandID::TIMER, RelayID::ALL, as_number(impl_::TimerDelayType::REMAINING));
    enqueueCommand(CommandID::JUMPER_STATUS);
    enqueueCommand(CommandID::FIRMWARE_VERSION);
}


/*!
    \brief Disconnects card.

    The method disconnects card, releases serial port and emits K8090::disconnected().
*/
void K8090::disconnect()
{
    doDisconnect();
    emit disconnected();
}


/*!
    \brief Refreshes info about card and relay states.

    Emitins K8090::relayStatus(), K8090::buttonModes(), K8090::totalTimerDelay(), K8090::remainingTimerDelay(),
    K8090::jumperStatus() and K8090::firmwareVersion() signals.
*/
void K8090::refreshRelaysInfo()
{
    enqueueCommand(CommandID::QUERY_RELAY);
    enqueueCommand(CommandID::BUTTON_MODE);
    enqueueCommand(CommandID::TIMER, RelayID::ALL, as_number(impl_::TimerDelayType::TOTAL));
    enqueueCommand(CommandID::TIMER, RelayID::ALL, as_number(impl_::TimerDelayType::REMAINING));
    enqueueCommand(CommandID::JUMPER_STATUS);
    enqueueCommand(CommandID::FIRMWARE_VERSION);
}


/*!
    \brief Switches specified relays on.

    If some button states is modified, the K8090::relayStatus() signal will be emited. If some
    k8090::CommandID::RELAY_OFF command is pending for execution, the relays required by this command are
    excluded from it.

    \param relays The relays.
    \sa K8090::relayStatus(), K8090::startRelayTimer()
*/
void K8090::switchRelayOn(RelayID relays)
{
    sendCommand(CommandID::RELAY_ON, relays);
}


/*!
    \brief Switches specified relays off.

    If some button states is modified, the K8090::relayStatus() signal will be emited. If some
    k8090::CommandID::RELAY_ON command is pending for execution, the relays required by this command are
    excluded from it.

    \param relays The relays.
    \sa K8090::relayStatus(), K8090::startRelayTimer()
*/
void K8090::switchRelayOff(RelayID relays)
{
    sendCommand(CommandID::RELAY_OFF, relays);
}


/*!
    \brief Toggles specified relays.

    The K8090::relayStatus() signal will be emited after command execution.

    \param relays The relays.
    \sa K8090::relayStatus(), K8090::startRelayTimer()
*/
void K8090::toggleRelay(RelayID relays)
{
    sendCommand(CommandID::TOGGLE_RELAY, relays);
}


/*!
    \brief Sets button modes.

    Configures the modes of each button. Available modes are momentary, toggle, timed. In case of duplicate assignments
    momentary mode has priority over toggle mode, and toggle mode has priority over timed mode. See the Velleman %K8090
    card manual for more details. There is also feature which is not documented in Velleman %K8090 card manual. If you
    don't set any mode for the given button, the physical button is disabled. For example, if you set
    `momentary = k8090::RelayID::ONE`, `toggle = k8090::RelayID::NONE` and
    `timed = k8090::RelayID::NONE`, the physical button one will be in momentary mode and all the other buttons
    will be disabled.

    \param momentary Relays to be set to momentary mode.
    \param toggle Relays to be set to toggle mode.
    \param timed Relays to be set to timed mode.
    \sa K8090::queryButtonModes(), K8090::buttonModes().
*/
void K8090::setButtonMode(RelayID momentary, RelayID toggle, RelayID timed)
{
    sendCommand(CommandID::SET_BUTTON_MODE, momentary, as_number(toggle), as_number(timed));
}


/*!
    \brief Starts timers for specified relays.

    When the timer starts, the specified relays are switched on, when it elapses they are switched off. The timer is
    aborted when some other action influences the relay. If it is another timer, the timer is restarted from the
    beginning. If this command changes state of some relay, the K8090::relayStatus() signal is emited. The remaining
    delay can be queried by K8090::queryRemainingTimerDelay(). If the delay is set to zero, the default delay will be
    used (see K8090::setRelayTimerDelay()). See the Velleman %K8090 card manual for more details.

    \note If a timer is started immediately after the timer ellapse, its timeout time is usually usually shorter (the
    difference can be as high as 0.5 sec).

    \param relays The relays.
    \param delay Required delay in seconds or 0 for default delay.
    \sa K8090::setRelayTimerDelay(), K8090::relayStatus().
*/
void K8090::startRelayTimer(RelayID relays, quint16 delay)
{
    sendCommand(CommandID::START_TIMER, relays, highByte(delay), lowByte(delay));
}


/*!
    \brief Sets the default timer delays.
    \param relays The influenced relays.
    \param delay Required delay in seconds.
    \sa K8090::startRelayTimer().
*/
void K8090::setRelayTimerDelay(RelayID relays, quint16 delay)
{
    sendCommand(CommandID::SET_TIMER, relays, highByte(delay), lowByte(delay));
}


/*!
    \brief Queries relay statuses.

    The K8090::relayStatus() signal is emited as the reaction.

    \sa K8090::refreshRelaysInfo(), K8090::switchRelayOn(), K8090::switchRelayOff(), K8090::toggleRelay()
    K8090::startRelayTimer()
*/
void K8090::queryRelayStatus()
{
    sendCommand(CommandID::QUERY_RELAY);
}


/*!
    \brief Queries total timer delays.

    The K8090::totalTimerDelay() signal is emited for each queried relay.

    \param relays The queried relays.
    \sa K8090::startRelayTimer(), K8090::queryRemainingTimerDelay()
*/
void K8090::queryTotalTimerDelay(RelayID relays)
{
    sendCommand(CommandID::TIMER, relays, as_number(impl_::TimerDelayType::TOTAL));
}


/*!
    \brief Queries remaining timer delays.

    The K8090::remainingTimerDelay() signal is emited for each queried relay.

    \param relays The queried relays.
    \sa K8090::startRelayTimer(), K8090::queryTotalTimerDelay()
*/
void K8090::queryRemainingTimerDelay(RelayID relays)
{
    sendCommand(CommandID::TIMER, relays, as_number(impl_::TimerDelayType::REMAINING));
}


/*!
    \brief Queries button modes.

    The K8090::buttonModes() signal with button is emited after the response from the card is received.

    \sa K8090::setButtonMode()
*/
void K8090::queryButtonModes()
{
    sendCommand(CommandID::BUTTON_MODE);
}


/*!
    \brief Resets card to factory defaults.

    Sets all buttons off and to toggle mode and all timer delays to 5 seconds. If some button states is modified, the
    K8090::relayStatus() signal will be emited.

    \sa K8090::setButtonMode() \sa K8090::setRelayTimerDelay()
*/
void K8090::resetFactoryDefaults()
{
    sendCommand(CommandID::RESET_FACTORY_DEFAULTS);
}


/*!
    \brief Queries jumper status.

    The K8090::jumperStatus() signal with ‘event’ jumper is emited after the response from the card is received.
*/
void K8090::queryJumperStatus()
{
    sendCommand(CommandID::JUMPER_STATUS);
}


/*!
    \brief Queries firmware version.

    The K8090::firmwareVersion() signal with firmware version is emited after the response from the card is received.
*/
void K8090::queryFirmwareVersion()
{
    sendCommand(CommandID::FIRMWARE_VERSION);
}


// private slots

// Reaction on received data from the card.
void K8090::onReadyData()
{
    QByteArray data = serial_port_->readAll();
    int n = data.size();
    for (int i = 0; i < n; i += 7) {
        if (n - i < 7) {
            onCommandFailed();
            return;
        } else {
            // TODO(lumik): switch to PIMPL and remove unnecessary heap usage
            std::unique_ptr<impl_::CardMessage> response;
            try {
                response.reset(new impl_::CardMessage{data.constBegin() + i, data.constBegin() + i + 7});
            } catch (const std::out_of_range &) {
                onCommandFailed();
                return;
            }
            if (!response->isValid()) {
                onCommandFailed();
                return;
            }
            switch (response->commandByte()) {
                case impl_::kResponses[as_number(ResponseID::BUTTON_MODE)] :
                    buttonModeResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::TIMER)] :
                    timerResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::BUTTON_STATUS)] :
                    buttonStatusResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::RELAY_STATUS)] :
                    relayStatusResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::JUMPER_STATUS)] :
                    jumperStatusResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::FIRMWARE_VERSION)] :
                    firmwareVersionResponse(std::move(response));
                    break;
                default:
                    onCommandFailed();
            }
        }
    }
}


// There must be some delay between commands, so the commands are inserted inside queue and dequeued after
// command_delay_ miliseconds which is controlled by commad_timer_. Commands with a response are dequeued after the
// response comes.
void K8090::dequeueCommand()
{
    // commands without response sends after delay the appropriate query command to test connection.
    CommandID command_id = current_command_->id;
    current_command_->id = CommandID::NONE;
    switch (command_id) {
        case CommandID::RELAY_ON:
        case CommandID::RELAY_OFF:
        case CommandID::TOGGLE_RELAY:
        case CommandID::START_TIMER:
        case CommandID::RESET_FACTORY_DEFAULTS:
            sendCommandHelper(CommandID::QUERY_RELAY);
            return;
        case CommandID::SET_BUTTON_MODE:
            sendCommandHelper(CommandID::BUTTON_MODE);
            return;
        case CommandID::SET_TIMER:
            sendCommandHelper(CommandID::TIMER, static_cast<RelayID>(current_command_->params[0]), 0);
            return;
        default:
            break;
    }

    if (!pending_commands_->empty()) {
        impl_::Command command = pending_commands_->pop();
        sendCommandHelper(command.id, static_cast<RelayID>(command.params[0]), command.params[1], command.params[2]);
    }
}


void K8090::onCommandFailed()
{
    failure_timer_->stop();
    ++failure_counter_;
    if (failure_counter_ > failure_max_count_) {
        connected_ = false;
        connecting_ = false;
        serial_port_->close();
        emit connectionFailed();
    }
}


// general top level method which sends commands to card. It controlls, if the card is connected and then uses
// enqueuCommand().
void K8090::sendCommand(CommandID command_id, RelayID mask, unsigned char param1, unsigned char param2)
{
    if (!connected_) {
        emit notConnected();
        return;
    }
    enqueueCommand(command_id, mask, param1, param2);
}


// Each command is sended through this method. It controlls if the command can be sended directly or enqueued for
// delayed sending, because the virtual serial port interface of the card doesn't accept commands which are sended with
// too small delay in between them.
void K8090::enqueueCommand(CommandID command_id, RelayID mask, unsigned char param1, unsigned char param2)
{
    // Send command directly if it is sufficiently delayed from the previous one and there are no commands pending.
    if (!command_timer_->isActive() && current_command_->id == CommandID::NONE && pending_commands_->empty()) {
        sendCommandHelper(command_id, mask, param1, param2);
    } else {  // send command undirectly
        // TODO(lumik): don't insert query commands if set command with the same response is already inside
        // TODO(lumik): treat commands, which are directly sended better (avoid duplication)
        impl_::Command command{command_id, impl_::kPriorities[as_number(command_id)], as_number(mask), param1, param2};
        const QList<const impl_::Command *> & pending_command_list = pending_commands_->get(command_id);
        // if there is no command with the same id waiting
        if (pending_command_list.isEmpty()) {
            pending_commands_->push(command);
        // else try to update stored command and if it is not possible (updateCommand returns false), push it to the
        // queue
        } else if (!updateCommand(command, pending_command_list)) {
            pending_commands_->push(command, false);
        }

        // if the enqueued command was switch relay on or off command and there is the oposit command stored
        // TODO(lumik): test if updated oposite command doesn't update any relay and if it does, remove it from the
        // queue
        if (command_id == CommandID::RELAY_ON) {
            const QList<const impl_::Command *> & off_pending_command_list
                    = pending_commands_->get(CommandID::RELAY_OFF);
            if (!off_pending_command_list.isEmpty()) {
                updateCommand(command, off_pending_command_list);
            }
        } else if (command_id == CommandID::RELAY_OFF) {
            const QList<const impl_::Command *> & on_pending_command_list = pending_commands_->get(CommandID::RELAY_ON);
            if (!on_pending_command_list.isEmpty()) {
                updateCommand(command, on_pending_command_list);
            }
        }
    }
}


// helper method which updates already enqueued command
bool K8090::updateCommand(const impl_::Command &command, const QList<const impl_::Command *> &pending_command_list)
{
    // check if equal command is in pending command list
    int compatible_idx = pending_command_list.size();
    for (int i = 0; i < pending_command_list.size(); ++i) {
        if (pending_command_list[i]->isCompatible(command)) {
            compatible_idx = i;
            break;
        }
    }
    if (compatible_idx != pending_command_list.size()) {
        impl_::Command insert_command = *pending_command_list[compatible_idx];
        insert_command |= command;
        if (insert_command.priority < command.priority) {
            insert_command.priority = command.priority;
        }
        pending_commands_->updateCommand(compatible_idx, insert_command);
        return true;
    }
    return false;
}


// constructs command
void K8090::sendCommandHelper(CommandID command_id, RelayID mask, unsigned char param1, unsigned char param2)
{
    static const int n = 7;  // Number of command bytes.
    std::unique_ptr<unsigned char []> cmd = std::unique_ptr<unsigned char []>{new unsigned char[n]};
    cmd[0] = impl_::kStxByte;
    cmd[1] = impl_::kCommands[as_number(command_id)];
    cmd[2] = as_number(mask);
    cmd[3] = param1;
    cmd[4] = param2;
    cmd[5] = impl_::check_sum(cmd.get(), 5);
    cmd[6] = impl_::kEtxByte;
    // store current command for response testing. Commands with no response triggers query task after the command
    // timer elapses, see the dequeuCommand() method.
    current_command_->id = command_id;
    current_command_->params[0] = as_number(mask);
    current_command_->params[1] = param1;
    current_command_->params[2] = param2;
    // if command can be without response, do not start failure check, next command is sent when the responses for the
    // command is processed
    if (hasResponse(command_id)) {
        failure_timer_->start(failure_delay_);
        if (command_id == CommandID::QUERY_RELAY) {
            command_timer_->start(command_delay_);
        } else if (command_id == CommandID::TOGGLE_RELAY) {
            // relay status can be response to many situations so it is better to not rely on right response and rather
            // send the next command after command delays
            command_timer_->start(command_delay_);
        }
    // if there is some delay between commands specified and the command hasn't response, start the delay
    } else if (command_delay_) {
        if (command_id == CommandID::RESET_FACTORY_DEFAULTS) {
            // reset factory defaults execution takes longer
            command_timer_->start(factory_defaults_command_delay_);
        } else {
            command_timer_->start(command_delay_);
        }
    }
    sendToSerial(std::move(cmd), n);
}


// helper method distinguishing commands which have response
bool K8090::hasResponse(CommandID command_id)
{
    switch (command_id) {
        case CommandID::RELAY_ON :
        case CommandID::RELAY_OFF :
        case CommandID::SET_BUTTON_MODE :
        case CommandID::START_TIMER :
        case CommandID::SET_TIMER :
        case CommandID::RESET_FACTORY_DEFAULTS :
            return false;
        default:
            return true;
    }
}


// sends command to serial port
void K8090::sendToSerial(std::unique_ptr<unsigned char[]> buffer, int n)
{
    if (!serial_port_->isOpen()) {
        if (!serial_port_->open(QIODevice::ReadWrite)) {
            connected_ = false;
            failure_timer_->stop();
            command_timer_->stop();
            emit connectionFailed();
            return;
        }
    }
    serial_port_->write(reinterpret_cast<char*>(buffer.get()), n);
    serial_port_->flush();
}


// processes button mode response
void K8090::buttonModeResponse(std::unique_ptr<impl_::CardMessage> response)
{
    // button mode was not requested
    if (current_command_->id != CommandID::BUTTON_MODE) {
        onCommandFailed();
        return;
    }
    // query button mode has no parameters. It is satisfactory only to remove one button mode request from the list
    current_command_->id = CommandID::NONE;
    failure_timer_->stop();
    if (connected_) {
        emit buttonModes(static_cast<RelayID>(response->data[2]), static_cast<RelayID>(response->data[3]),
            static_cast<RelayID>(response->data[4]));
        dequeueCommand();
    } else if (connecting_) {
        emit buttonModes(static_cast<RelayID>(response->data[2]), static_cast<RelayID>(response->data[3]),
            static_cast<RelayID>(response->data[4]));
        if (pending_commands_->empty()) {
            connectionSuccessful();
        } else {
            dequeueCommand();
        }
    } else {
        // TODO(lumik): this should not occur. Convert it to exception.
        onCommandFailed();
    }
}


// processes timer response
void K8090::timerResponse(std::unique_ptr<impl_::CardMessage> response)
{
    // timer was not requested
    if (current_command_->id != CommandID::TIMER) {
        onCommandFailed();
        return;
    }
    bool is_total;
    bool should_dequeue_next = false;
    // total timer
    if (~(current_command_->params[1]) & (1 << 0)) {
        // remove current response from the list of waiting to response commands.
        is_total = true;
        current_command_->params[0] &= ~response->data[2];
        if (!current_command_->params[0]) {
            current_command_->id = CommandID::NONE;
            failure_timer_->stop();
            should_dequeue_next = true;
        } else {
            failure_timer_->start();
        }
    } else {
        is_total = false;
        current_command_->params[0] &= ~response->data[2];
        if (!current_command_->params[0]) {
            current_command_->id = CommandID::NONE;
            failure_timer_->stop();
            should_dequeue_next = true;
        } else {
            failure_timer_->start();
        }
    }
    if (connected_ | connecting_) {
        if (is_total) {
            emit totalTimerDelay(static_cast<RelayID>(response->data[2]), response->data[3] << 8 | response->data[4]);
        } else {
            emit remainingTimerDelay(static_cast<RelayID>(response->data[2]),
                    response->data[3] << 8 | response->data[4]);
        }
        if (connecting_ && pending_commands_->empty()) {
            connectionSuccessful();
        } else if (should_dequeue_next) {
            dequeueCommand();
        }
    } else {
        // TODO(lumik): this should not occur, convert it to exception.
        onCommandFailed();
    }
}


// processes button status response
void K8090::buttonStatusResponse(std::unique_ptr<impl_::CardMessage> response)
{
    if (connected_) {
        emit buttonStatus(static_cast<RelayID>(response->data[2]), static_cast<RelayID>(response->data[3]),
                static_cast<RelayID>(response->data[4]));
    }
    // button status is emited only after user interaction with physical buttons on the relay, no query command is
    // connected with it
}


// processes relay status response
void K8090::relayStatusResponse(std::unique_ptr<impl_::CardMessage> response)
{
    // relay status can be a response to many commands. If status changes by the command, it is not necessary to query
    if (current_command_->id == CommandID::QUERY_RELAY) {
        current_command_->id = CommandID::NONE;
        failure_timer_->stop();
    // switch relay on
    } else if (current_command_->id == CommandID::RELAY_ON) {
        // test if all required relays are on:
        bool match = true;
        for (int i = 0; i < 8; ++i) {
            if (current_command_->params[0] & (1 << i) & ~response->data[3]) {
                match = false;
            }
        }
        if (match) {
            current_command_->id = CommandID::NONE;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    // switch relay off
    } else if (current_command_->id == CommandID::RELAY_OFF) {
        // test if all required relays are off:
        bool match = true;
        for (int i = 0; i < 8; ++i) {
            if (current_command_->params[0] & (1 << i) & response->data[3]) {
                match = false;
            }
        }
        if (match) {
            current_command_->id = CommandID::NONE;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    } else if (current_command_->id == CommandID::TOGGLE_RELAY) {
        // TODO(lumik): consider the toggle relay testing
        current_command_->id = CommandID::NONE;
        failure_timer_->stop();
    } else if (current_command_->id == CommandID::START_TIMER) {
        // test if all required relays are on:
        bool match = true;
        for (int i = 0; i < 8; ++i) {
            if (current_command_->params[0] & (1 << i) & ~response->data[3]) {
                match = false;
            }
        }
        if (match) {
            current_command_->id = CommandID::NONE;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    } else if (current_command_->id == CommandID::RESET_FACTORY_DEFAULTS) {
        // test if all required relays are off:
        bool match = true;
        if (response->data[3]) {
            match = false;
        }
        if (match) {
            current_command_->id = CommandID::NONE;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    }
    if (connected_) {
        emit relayStatus(static_cast<RelayID>(response->data[2]), static_cast<RelayID>(response->data[3]),
                static_cast<RelayID>(response->data[4]));
    } else if (connecting_) {
        // Beware, if the relay status message is obtained from the card as the reaction to the user interaction with
        // physical buttons, the relay status signal can be emited 2 times because of the message obtained as the
        // reaction to query message.
        emit relayStatus(static_cast<RelayID>(response->data[2]), static_cast<RelayID>(response->data[3]),
                static_cast<RelayID>(response->data[4]));
        if (pending_commands_->empty()) {
            connectionSuccessful();
        }
    }
    // relay status can be also emited in reaction to on, off, toggle, start timer commands or physical button press,
    // so it is better to leave the next command sending to timer. So it is treated in sendCommandHelper() method.
}


// processes jumper status response
void K8090::jumperStatusResponse(std::unique_ptr<impl_::CardMessage> response)
{
    if (current_command_->id != CommandID::JUMPER_STATUS) {
        onCommandFailed();
        return;
    }
    current_command_->id = CommandID::NONE;
    failure_timer_->stop();
    if (connected_) {
        emit jumperStatus(static_cast<bool>(response->data[3]));
        dequeueCommand();
    } else if (connecting_) {
        emit jumperStatus(static_cast<bool>(response->data[3]));
        if (pending_commands_->empty()) {
            connectionSuccessful();
        } else {
            dequeueCommand();
        }
    } else {
        // TODO(lumik): this should not occur, convert it to exception.
        onCommandFailed();
    }
}


// processes firmware version response
void K8090::firmwareVersionResponse(std::unique_ptr<impl_::CardMessage> response)
{
    if (current_command_->id != CommandID::FIRMWARE_VERSION) {
        onCommandFailed();
        return;
    }
    current_command_->id = CommandID::NONE;
    failure_timer_->stop();
    if (connected_) {
        emit firmwareVersion(2000 + static_cast<int>(response->data[3]), static_cast<int>(response->data[4]));
        dequeueCommand();
    } else if (connecting_) {
        emit firmwareVersion(2000 + static_cast<int>(response->data[3]), static_cast<int>(response->data[4]));
        if (pending_commands_->empty()) {
            connectionSuccessful();
        } else {
            dequeueCommand();
        }
    } else {
        // TODO(lumik): this should not occur, convert it to exception.
        onCommandFailed();
    }
}


// This method should be called at the end of connection process to set class to connected state and notify user about
// that.
void K8090::connectionSuccessful()
{
    connecting_ = false;
    connected_ = true;
    emit connected();
}


// This method should be called to do all the stuff needed to disconnect from the relay card
void K8090::doDisconnect()
{
    serial_port_->close();
    // erase all pending commands
    pending_commands_.reset(new command_queue::CommandQueue<impl_::Command, as_number(CommandID::NONE)>);
    // stop failure timers and erase failure counter
    command_timer_->stop();
    failure_timer_->stop();
    failure_counter_ = 0;

    connected_ = false;
    connecting_ = false;
}

}  // namespace k8090
}  // namespace core
}  // namespace sprelay
