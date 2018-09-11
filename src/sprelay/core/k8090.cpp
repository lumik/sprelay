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

#include <QMutex>
#include <QStringBuilder>
#include <QTimer>

#include <stdexcept>
#include <utility>

#include "command_queue.h"
#include "concurent_command_queue.h"
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
    com_port_name_mutex_{new QMutex},
    serial_port_{new UnifiedSerialPort},
    pending_commands_{new impl_::ConcurentCommandQueue},
    current_command_{new impl_::Command},
    command_timer_{new QTimer},
    failure_timer_{new QTimer},
    failure_counter_{0},
    connected_{false},
    connecting_{false},
    connected_mutex_{new QMutex},
    command_delay_{kDefaultCommandDelay_},
    factory_defaults_command_delay_{2 * kDefaultCommandDelay_},
    command_delay_mutex_{new QMutex},
    failure_delay_{kDefaultFailureDelay_},
    failure_delay_mutex_{new QMutex},
    failure_max_count_{kDefaultMaxFailureCount_},
    failure_max_count_mutex_{new QMutex}
{
    command_timer_->setSingleShot(true);
    failure_timer_->setSingleShot(true);

    connect(serial_port_.get(), &UnifiedSerialPort::readyRead, this, &K8090::onReadyData);
    connect(command_timer_.get(), &QTimer::timeout, this, &K8090::dequeueCommand);
    connect(failure_timer_.get(), &QTimer::timeout, this, &K8090::onCommandFailed);
    connect(this, &K8090::doDisconnect, this, &K8090::onDoDisconnect);
    connect(this, static_cast<void (K8090::*)(CommandID)>(&K8090::enqueueCommand),
        this, [=](CommandID command_id) { this->onEnqueueCommand(command_id); });
    connect(this, static_cast<void (K8090::*)(CommandID, RelayID)>(&K8090::enqueueCommand),
        this, [=](CommandID command_id, RelayID mask) { this->onEnqueueCommand(command_id, mask); });
    connect(this, static_cast<void (K8090::*)(CommandID, RelayID, unsigned char)>(&K8090::enqueueCommand),
        this, [=](CommandID command_id, RelayID mask, unsigned char param1)
            { this->onEnqueueCommand(command_id, mask, param1); });
    connect(this, static_cast<void (K8090::*)(CommandID, RelayID, unsigned char, unsigned char)>(
            &K8090::enqueueCommand),
        this, [=](CommandID command_id, RelayID mask, unsigned char param1, unsigned char param2)
            { this->onEnqueueCommand(command_id, mask, param1, param2); });
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
  \remark reentrant, thread-safe.
*/
QList<serial_utils::ComPortParams> K8090::availablePorts()
{
    // UnifiedSerialPort::availablePorts is thread safe, so it is safe to call it
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
    QMutexLocker com_port_name_locker{com_port_name_mutex_.get()};
    if (com_port_name_ != name) {
        com_port_name_ = name;
        com_port_name_locker.unlock();
        emit doDisconnect(false);
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
    QMutexLocker command_delay_locker{command_delay_mutex_.get()};
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
    QMutexLocker failure_delay_locker{failure_delay_mutex_.get()};
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
    QMutexLocker failure_max_count_locker{failure_max_count_mutex_.get()};
    failure_max_count_ = count;
}


/*!
    \brief Test if the relay is connected.
    \return True if connected.
*/
bool K8090::isConnected()
{
    QMutexLocker connected_locker{connected_mutex_.get()};
    return connected_;
}


/*!
    \brief Gets a number of commands waiting for execution in queue.
    \param id Queried command.
    \return The number of commands in queue.(
*/
int K8090::pendingCommandCount(CommandID id)
{
    // TODO(lumik): Replace this hack. Pending commands unique_ptr is reseted in doDisconnect method when
    // connected_mutex_ is locked. Create new reset method of ConcurentCommandQueue and use it here instead.
    QMutexLocker connected_locker{connected_mutex_.get()};
    return pending_commands_->count(id);
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
/*!
    \fn void K8090::doDisconnect(bool failure)
    \brief A signal for internal usage to disconnect in K8090's thread.
*/
/*!
    \fn void K8090::enqueueCommand(sprelay::core::k8090::CommandID command_id)
    \brief A signal for internal usage to enqueueCommand in K8090's thread.
*/
/*!
    \fn void K8090::enqueueCommand(sprelay::core::k8090::CommandID command_id, sprelay::core::k8090::RelayID mask)
    \brief A signal for internal usage to enqueueCommand in K8090's thread.
*/
/*!
    \fn void K8090::enqueueCommand(sprelay::core::k8090::CommandID command_id, sprelay::core::k8090::RelayID mask,
        unsigned char param1)
    \brief A signal for internal usage to enqueueCommand in K8090's thread.
*/
/*!
    \fn void K8090::enqueueCommand(sprelay::core::k8090::CommandID command_id, sprelay::core::k8090::RelayID mask,
        unsigned char param1, unsigned char param2)
    \brief A signal for internal usage to enqueueCommand in K8090's thread.
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
    QMutexLocker connected_locker{connected_mutex_.get()};
    if (connecting_) {
        return;
    }
    connected_ = false;
    bool card_found = false;
    QMutexLocker com_port_name_locker{com_port_name_mutex_.get()};
    foreach (const serial_utils::ComPortParams &params,  // NOLINT(whitespace/parens)
            UnifiedSerialPort::availablePorts()) {
        if (params.port_name == com_port_name_
                && params.product_identifier == kProductID
                && params.vendor_identifier == kVendorID) {
            card_found = true;
        }
    }
    com_port_name_locker.unlock();

    if (!card_found) {
        connected_locker.unlock();
        emit connectionFailed();
        return;
    }

    com_port_name_locker.relock();
    serial_port_->setPortName(com_port_name_);
    com_port_name_locker.unlock();
    serial_port_->setBaudRate(QSerialPort::Baud19200);
    serial_port_->setDataBits(QSerialPort::Data8);
    serial_port_->setParity(QSerialPort::NoParity);
    serial_port_->setStopBits(QSerialPort::OneStop);
    serial_port_->setFlowControl(QSerialPort::NoFlowControl);

    if (!serial_port_->isOpen()) {
        if (!serial_port_->open(QIODevice::ReadWrite)) {
            connected_locker.unlock();
            emit connectionFailed();
            return;
        }
    }

    connecting_ = true;
    connected_locker.unlock();

    emit enqueueCommand(CommandID::QueryRelay);
    emit enqueueCommand(CommandID::ButtonMode);
    emit enqueueCommand(CommandID::Timer, RelayID::All, as_number(impl_::TimerDelayType::Total));
    emit enqueueCommand(CommandID::Timer, RelayID::All, as_number(impl_::TimerDelayType::Remaining));
    emit enqueueCommand(CommandID::JumperStatus);
    emit enqueueCommand(CommandID::FirmwareVersion);
}


/*!
    \brief Disconnects card.

    The method disconnects card, releases serial port and emits K8090::disconnected().
*/
void K8090::disconnect()
{
    emit doDisconnect(false);
}


/*!
    \brief Refreshes info about card and relay states.

    Emitins K8090::relayStatus(), K8090::buttonModes(), K8090::totalTimerDelay(), K8090::remainingTimerDelay(),
    K8090::jumperStatus() and K8090::firmwareVersion() signals.
*/
void K8090::refreshRelaysInfo()
{
    emit enqueueCommand(CommandID::QueryRelay);
    emit enqueueCommand(CommandID::ButtonMode);
    emit enqueueCommand(CommandID::Timer, RelayID::All, as_number(impl_::TimerDelayType::Total));
    emit enqueueCommand(CommandID::Timer, RelayID::All, as_number(impl_::TimerDelayType::Remaining));
    emit enqueueCommand(CommandID::JumperStatus);
    emit enqueueCommand(CommandID::FirmwareVersion);
}


/*!
    \brief Switches specified relays on.

    If some button states is modified, the K8090::relayStatus() signal will be emited. If some
    k8090::CommandID::RelayOff command is pending for execution, the relays required by this command are
    excluded from it.

    \param relays The relays.
    \sa K8090::relayStatus(), K8090::startRelayTimer()
*/
void K8090::switchRelayOn(RelayID relays)
{
    sendCommand(CommandID::RelayOn, relays);
}


/*!
    \brief Switches specified relays off.

    If some button states is modified, the K8090::relayStatus() signal will be emited. If some
    k8090::CommandID::RelayOn command is pending for execution, the relays required by this command are
    excluded from it.

    \param relays The relays.
    \sa K8090::relayStatus(), K8090::startRelayTimer()
*/
void K8090::switchRelayOff(RelayID relays)
{
    sendCommand(CommandID::RelayOff, relays);
}


/*!
    \brief Toggles specified relays.

    The K8090::relayStatus() signal will be emited after command execution.

    \param relays The relays.
    \sa K8090::relayStatus(), K8090::startRelayTimer()
*/
void K8090::toggleRelay(RelayID relays)
{
    sendCommand(CommandID::ToggleRelay, relays);
}


/*!
    \brief Sets button modes.

    Configures the modes of each button. Available modes are momentary, toggle, timed. In case of duplicate assignments
    momentary mode has priority over toggle mode, and toggle mode has priority over timed mode. See the Velleman %K8090
    card manual for more details. There is also feature which is not documented in Velleman %K8090 card manual. If you
    don't set any mode for the given button, the physical button is disabled. For example, if you set
    `momentary = k8090::RelayID::One`, `toggle = k8090::RelayID::None` and
    `timed = k8090::RelayID::None`, the physical button one will be in momentary mode and all the other buttons
    will be disabled.

    \param momentary Relays to be set to momentary mode.
    \param toggle Relays to be set to toggle mode.
    \param timed Relays to be set to timed mode.
    \sa K8090::queryButtonModes(), K8090::buttonModes().
*/
void K8090::setButtonMode(RelayID momentary, RelayID toggle, RelayID timed)
{
    sendCommand(CommandID::SetButtonMode, momentary, as_number(toggle), as_number(timed));
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
    sendCommand(CommandID::StartTimer, relays, highByte(delay), lowByte(delay));
}


/*!
    \brief Sets the default timer delays.
    \param relays The influenced relays.
    \param delay Required delay in seconds.
    \sa K8090::startRelayTimer().
*/
void K8090::setRelayTimerDelay(RelayID relays, quint16 delay)
{
    sendCommand(CommandID::SetTimer, relays, highByte(delay), lowByte(delay));
}


/*!
    \brief Queries relay statuses.

    The K8090::relayStatus() signal is emited as the reaction.

    \sa K8090::refreshRelaysInfo(), K8090::switchRelayOn(), K8090::switchRelayOff(), K8090::toggleRelay()
    K8090::startRelayTimer()
*/
void K8090::queryRelayStatus()
{
    sendCommand(CommandID::QueryRelay);
}


/*!
    \brief Queries total timer delays.

    The K8090::totalTimerDelay() signal is emited for each queried relay.

    \param relays The queried relays.
    \sa K8090::startRelayTimer(), K8090::queryRemainingTimerDelay()
*/
void K8090::queryTotalTimerDelay(RelayID relays)
{
    sendCommand(CommandID::Timer, relays, as_number(impl_::TimerDelayType::Total));
}


/*!
    \brief Queries remaining timer delays.

    The K8090::remainingTimerDelay() signal is emited for each queried relay.

    \param relays The queried relays.
    \sa K8090::startRelayTimer(), K8090::queryTotalTimerDelay()
*/
void K8090::queryRemainingTimerDelay(RelayID relays)
{
    sendCommand(CommandID::Timer, relays, as_number(impl_::TimerDelayType::Remaining));
}


/*!
    \brief Queries button modes.

    The K8090::buttonModes() signal with button is emited after the response from the card is received.

    \sa K8090::setButtonMode()
*/
void K8090::queryButtonModes()
{
    sendCommand(CommandID::ButtonMode);
}


/*!
    \brief Resets card to factory defaults.

    Sets all buttons off and to toggle mode and all timer delays to 5 seconds. If some button states is modified, the
    K8090::relayStatus() signal will be emited.

    \sa K8090::setButtonMode() \sa K8090::setRelayTimerDelay()
*/
void K8090::resetFactoryDefaults()
{
    sendCommand(CommandID::ResetFactoryDefaults);
}


/*!
    \brief Queries jumper status.

    The K8090::jumperStatus() signal with ‘event’ jumper is emited after the response from the card is received.
*/
void K8090::queryJumperStatus()
{
    sendCommand(CommandID::JumperStatus);
}


/*!
    \brief Queries firmware version.

    The K8090::firmwareVersion() signal with firmware version is emited after the response from the card is received.
*/
void K8090::queryFirmwareVersion()
{
    sendCommand(CommandID::FirmwareVersion);
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
                case impl_::kResponses[as_number(ResponseID::ButtonMode)] :
                    buttonModeResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::Timer)] :
                    timerResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::ButtonStatus)] :
                    buttonStatusResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::RelayStatus)] :
                    relayStatusResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::JumperStatus)] :
                    jumperStatusResponse(std::move(response));
                    break;
                case impl_::kResponses[as_number(ResponseID::FirmwareVersion)] :
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
// dequeueCommand() is alway called from the K8090's thread.
void K8090::dequeueCommand()
{
    // commands without response sends after delay the appropriate query command to test connection.
    CommandID command_id = current_command_->id;
    current_command_->id = CommandID::None;
    switch (command_id) {
        case CommandID::RelayOn:
        case CommandID::RelayOff:
        case CommandID::ToggleRelay:
        case CommandID::StartTimer:
        case CommandID::ResetFactoryDefaults:
            sendCommandHelper(CommandID::QueryRelay);
            return;
        case CommandID::SetButtonMode:
            sendCommandHelper(CommandID::ButtonMode);
            return;
        case CommandID::SetTimer:
            sendCommandHelper(CommandID::Timer, static_cast<RelayID>(current_command_->params[0]), 0);
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
    if (failure_counter_ > (QMutexLocker{failure_max_count_mutex_.get()}, failure_max_count_)) {
        onDoDisconnect(true);
    }
}


// This method should be called to do all the stuff needed to disconnect from the relay card
// Stops timer, so it should be run in the K8090's thread. It is achieved by calling this method through signal slots
// mechanism
void K8090::onDoDisconnect(bool failure)
{
    QMutexLocker connected_locker{connected_mutex_.get()};
    if (connected_ | connecting_) {
        serial_port_->close();
        // erase all pending commands
        pending_commands_.reset(new impl_::ConcurentCommandQueue);
        // stop failure timers and erase failure counter
        command_timer_->stop();
        failure_timer_->stop();
        failure_counter_ = 0;

        connected_ = false;
        connecting_ = false;

        connected_locker.unlock();

        if (failure) {
            emit connectionFailed();
        } else {
            emit disconnected();
        }
    }
}


// general top level method which sends commands to card. It controlls, if the card is connected and then uses
// enqueuCommand().
void K8090::sendCommand(CommandID command_id, RelayID mask, unsigned char param1, unsigned char param2)
{
    if (QMutexLocker{connected_mutex_.get()}, !connected_) {
        emit notConnected();
        return;
    }
    emit enqueueCommand(command_id, mask, param1, param2);
}


// Each command is sended through this method. It controlls if the command can be sended directly or enqueued for
// delayed sending, because the virtual serial port interface of the card doesn't accept commands which are sended with
// too small delay in between them.
// This method must be used from the K8090's thread, so invoke it by emitting enqueCommand signal through lambda
// expression - see connections in the constructor.
void K8090::onEnqueueCommand(CommandID command_id, RelayID mask, unsigned char param1, unsigned char param2)
{
    // Send command directly if it is sufficiently delayed from the previous one and there are no commands pending.
    if ((!command_timer_->isActive()) && current_command_->id == CommandID::None && pending_commands_->empty()) {
        sendCommandHelper(command_id, mask, param1, param2);
    } else {  // send command undirectly
        pending_commands_->updateOrPush(command_id, mask, param1, param2);
    }
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
        failure_timer_->start((QMutexLocker{failure_delay_mutex_.get()}, failure_delay_));
        if (command_id == CommandID::QueryRelay) {
            command_timer_->start((QMutexLocker{command_delay_mutex_.get()}, command_delay_));
        } else if (command_id == CommandID::ToggleRelay) {
            // relay status can be response to many situations so it is better to not rely on right response and rather
            // send the next command after command delays
            command_timer_->start((QMutexLocker{command_delay_mutex_.get()}, command_delay_));
        }
    // if there is some delay between commands specified and the command hasn't response, start the delay
    } else if (QMutexLocker{command_delay_mutex_.get()}, command_delay_) {
        if (command_id == CommandID::ResetFactoryDefaults) {
            // reset factory defaults execution takes longer
            command_timer_->start((QMutexLocker{command_delay_mutex_.get()}, factory_defaults_command_delay_));
        } else {
            command_timer_->start((QMutexLocker{command_delay_mutex_.get()}, command_delay_));
        }
    }
    sendToSerial(std::move(cmd), n);
}


// helper method distinguishing commands which have response
bool K8090::hasResponse(CommandID command_id)
{
    switch (command_id) {
        case CommandID::RelayOn :
        case CommandID::RelayOff :
        case CommandID::SetButtonMode :
        case CommandID::StartTimer :
        case CommandID::SetTimer :
        case CommandID::ResetFactoryDefaults :
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
            onDoDisconnect(true);
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
    if (current_command_->id != CommandID::ButtonMode) {
        onCommandFailed();
        return;
    }
    // query button mode has no parameters. It is satisfactory only to remove one button mode request from the list
    current_command_->id = CommandID::None;
    failure_timer_->stop();
    if (QMutexLocker{connected_mutex_.get()}, connected_) {
        emit buttonModes(static_cast<RelayID>(response->data[2]), static_cast<RelayID>(response->data[3]),
            static_cast<RelayID>(response->data[4]));
        dequeueCommand();
    } else if (QMutexLocker {connected_mutex_.get()}, connecting_) {
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
    if (current_command_->id != CommandID::Timer) {
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
            current_command_->id = CommandID::None;
            failure_timer_->stop();
            should_dequeue_next = true;
        } else {
            failure_timer_->start();
        }
    } else {
        is_total = false;
        current_command_->params[0] &= ~response->data[2];
        if (!current_command_->params[0]) {
            current_command_->id = CommandID::None;
            failure_timer_->stop();
            should_dequeue_next = true;
        } else {
            failure_timer_->start();
        }
    }
    if (QMutexLocker{connected_mutex_.get()}, (connected_ | connecting_)) {
        if (is_total) {
            emit totalTimerDelay(static_cast<RelayID>(response->data[2]), response->data[3] << 8 | response->data[4]);
        } else {
            emit remainingTimerDelay(static_cast<RelayID>(response->data[2]),
                    response->data[3] << 8 | response->data[4]);
        }
        if ((QMutexLocker{connected_mutex_.get()}, connecting_) && pending_commands_->empty()) {
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
    if (QMutexLocker{connected_mutex_.get()}, connected_) {
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
    if (current_command_->id == CommandID::QueryRelay) {
        current_command_->id = CommandID::None;
        failure_timer_->stop();
    // switch relay on
    } else if (current_command_->id == CommandID::RelayOn) {
        // test if all required relays are on:
        bool match = true;
        for (int i = 0; i < 8; ++i) {
            if (current_command_->params[0] & (1 << i) & ~response->data[3]) {
                match = false;
            }
        }
        if (match) {
            current_command_->id = CommandID::None;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    // switch relay off
    } else if (current_command_->id == CommandID::RelayOff) {
        // test if all required relays are off:
        bool match = true;
        for (int i = 0; i < 8; ++i) {
            if (current_command_->params[0] & (1 << i) & response->data[3]) {
                match = false;
            }
        }
        if (match) {
            current_command_->id = CommandID::None;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    } else if (current_command_->id == CommandID::ToggleRelay) {
        // TODO(lumik): consider the toggle relay testing
        current_command_->id = CommandID::None;
        failure_timer_->stop();
    } else if (current_command_->id == CommandID::StartTimer) {
        // test if all required relays are on:
        bool match = true;
        for (int i = 0; i < 8; ++i) {
            if (current_command_->params[0] & (1 << i) & ~response->data[3]) {
                match = false;
            }
        }
        if (match) {
            current_command_->id = CommandID::None;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    } else if (current_command_->id == CommandID::ResetFactoryDefaults) {
        // test if all required relays are off:
        bool match = true;
        if (response->data[3]) {
            match = false;
        }
        if (match) {
            current_command_->id = CommandID::None;
        }
        // TODO(lumik): think of testing, if the command was realy satisfied but beware of command merging by the card
        // or user interaction directly with the card
        failure_timer_->stop();
    }
    if (QMutexLocker{connected_mutex_.get()}, connected_) {
        emit relayStatus(static_cast<RelayID>(response->data[2]), static_cast<RelayID>(response->data[3]),
                static_cast<RelayID>(response->data[4]));
    } else if (QMutexLocker{connected_mutex_.get()}, connecting_) {
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
    if (current_command_->id != CommandID::JumperStatus) {
        onCommandFailed();
        return;
    }
    current_command_->id = CommandID::None;
    failure_timer_->stop();
    if (QMutexLocker{connected_mutex_.get()}, connected_) {
        emit jumperStatus(static_cast<bool>(response->data[3]));
        dequeueCommand();
    } else if (QMutexLocker{connected_mutex_.get()}, connecting_) {
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
    if (current_command_->id != CommandID::FirmwareVersion) {
        onCommandFailed();
        return;
    }
    current_command_->id = CommandID::None;
    failure_timer_->stop();
    if (QMutexLocker{connected_mutex_.get()}, connected_) {
        emit firmwareVersion(2000 + static_cast<int>(response->data[3]), static_cast<int>(response->data[4]));
        dequeueCommand();
    } else if (QMutexLocker{connected_mutex_.get()}, connecting_) {
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
    {
        QMutexLocker connected_locker{connected_mutex_.get()};
        connecting_ = false;
        connected_ = true;
    }
    emit connected();
}

}  // namespace k8090
}  // namespace core
}  // namespace sprelay
