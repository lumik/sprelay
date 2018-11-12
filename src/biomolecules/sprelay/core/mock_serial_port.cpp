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
 * \file      mock_serial_port.cpp
 * \brief     The biomolecules::sprelay::core::MockSerialPort class which implements the simulated relay card.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "mock_serial_port.h"

#include <cstdint>
#include <limits>
#include <random>
#include <utility>

#ifdef __MINGW32__  // random device on MinGW does not work, a seed is always same so we need chrono for seed.
#include <chrono>
#endif

#include "k8090_commands.h"
#include "k8090_utils.h"

namespace biomolecules {
namespace sprelay {
namespace core {


// generator of random numbers which is used throughout the class to produce random undefined responses (active timer
// delay query when the timer is not active) and random response delays from the card

// random device on MinGW does not work, a seed is always same.
#ifdef __MINGW32__
static std::mt19937_64 random_generator{
    static_cast<std::uint_fast64_t>(std::chrono::system_clock::now().time_since_epoch().count())};
#else   // ifdef __MINGW32__
static std::random_device random_device;
static std::mt19937_64 random_generator{random_device()};
#endif  // ifdef __MINGW32__


/*!
 * \class MockSerialPort
 * Communication with the card is performed through the ability of windows to make virtual COM port from USB. This
 * class tries to behavior as if it was the real card connected through this virtual COM port interface. The port
 * parameters should be set as follows:
 * parameter     | value
 * ------------- | ----------------------------
 * port name     | redundant, can be anything
 * baud rate     | `QSerialPort::Baud19200`
 * data bits     | `QSerialPort::Data8`
 * parity        | `QSerialPort::NoParity`
 * stop bits     | `QSerialPort::OneStop`
 * flow control  | `QSerialPort::NoFlowControl`
 *
 * The class is used in the same way as the `QSerialPort` class, so you have to MockSerialPort::open() the port before
 * you can MockSerialPort::write() data to it. When simulated data comes from the port, the
 * MockSerialPort::readyRead() signal is emited. You can read the data by MockSerialPort::readAll() method then. The
 * port can be tested, if it is open by MockSerialPort::isOpen() method and when the communication with the port ends,
 * the port can be closed by MockSerialPort::close() method.
 *
 * The communication is event-driven, polling is not necessary. The communication packets are 7 bytes in size, but due
 * to delays in communication received packets can accumulate so the response from the card can be multiples of 7
 * bytes long. See the Velleman relay %K8090 card manual for more information about communication packes scheme and
 * form of patricular commands.
 *
 * Some commands can trigger response from the card. To simulate real card, these responses are randomly delayed
 * between 5 and 25 ms. If you start timer on the real card right after the timer elapsed, the timers timeout is less
 * than the required about 500ms. This behavior is not mimicked in this mock class. Command also cant be sended too
 * close to each other in the real card. For different commands, the required delay is different but rough upper
 * estimate is 50 ms. This behavior is also not implemented in the mock class.
 *
 * When the timers has approximately same time of timeout, their timeout is merged together. This behavior of rela
 * card is used also here. When all timers with timeouts less then 100 ms in the time some timer times out are also
 * timed out in one step with the currently timed out timer.
 *
 * \remark reentrant
 * \sa MockSerialPort::setBaudRate(), MockSerialPort::setDataBits(), MockSerialPort::setParity(),
 * MockSerialPort::setStopBits(), MockSerialPort::setFlowControl()
 */

// initialization of static member variables

// public
/*!
 * \brief Product id for port identification. Taken from real card obtained at FUUK.
 */
const quint16 MockSerialPort::kProductID = k8090::impl_::kProductID;

/*!
 * \brief Vendor id for port identification. Taken from real card obtained at FUUK.
 */
const quint16 MockSerialPort::kVendorID = k8090::impl_::kVendorID;

// private
// minimal delay for random response delay generation
const int MockSerialPort::kMinResponseDelayMs_ = 2;
// maximal delay for random response delay generation
const int MockSerialPort::kMaxResponseDelayMs_ = 10;
// probability of success of response delay binomial distribution
const float MockSerialPort::kResponseDelayDistributionP = 0.3f;
// Serial port settings
const qint32 MockSerialPort::kNeededBaudRate_ = QSerialPort::Baud19200;
const QSerialPort::DataBits MockSerialPort::kNeededDataBits_ = QSerialPort::Data8;
const QSerialPort::Parity MockSerialPort::kNeededParity_ = QSerialPort::NoParity;
const QSerialPort::StopBits MockSerialPort::kNeedeStopBits_ = QSerialPort::OneStop;
const QSerialPort::FlowControl MockSerialPort::kNeededFlowControl_ = QSerialPort::NoFlowControl;
// Timers with this remaining time difference will timeout at the same time
const int MockSerialPort::kTimerDeltaMs_ = 100;


/*!
 * \brief The constructor.
 * \param parent Parent object in Qt ownership system.
 */
MockSerialPort::MockSerialPort(QObject* parent)
    : QObject{parent},
      baud_rate_{QSerialPort::Baud9600},
      data_bits_{QSerialPort::Data8},
      parity_{QSerialPort::NoParity},
      stop_bits_{QSerialPort::OneStop},
      flow_control_{QSerialPort::NoFlowControl},
      error_{QSerialPort::NoError},
      open_{false},
      on_{k8090::as_number(k8090::RelayID::None)},
      momentary_{k8090::as_number(k8090::RelayID::None)},
      toggle_{k8090::as_number(k8090::RelayID::All)},
      timed_{k8090::as_number(k8090::RelayID::None)},
      pressed_{k8090::as_number(k8090::RelayID::None)},
      default_delays_{5, 5, 5, 5, 5, 5, 5, 5},
      active_timers_{k8090::as_number(k8090::RelayID::None)},
      jumper_status_{0},
      firmware_version_{16, 6},
      delay_timer_mapper_{new QSignalMapper}
{
    std::uniform_int_distribution<int> distribution{
        std::numeric_limits<quint16>::min(), std::numeric_limits<quint16>::max()};
    for (int i = 0; i < 8; ++i) {
        remaining_delays_[i] = distribution(random_generator);
        delay_timers_[i].setSingleShot(true);
        connect(&delay_timers_[i], &QTimer::timeout,  // wrap
            delay_timer_mapper_.get(), static_cast<void (QSignalMapper::*)()>(&QSignalMapper::map));
        delay_timer_mapper_->setMapping(&delay_timers_[i], i);
    }
    connect(delay_timer_mapper_.get(), static_cast<void (QSignalMapper::*)(int)>(&QSignalMapper::mapped),  // wrap
        this, &MockSerialPort::delayTimeout);
    response_timer_.setSingleShot(true);
    connect(&response_timer_, &QTimer::timeout, this, &MockSerialPort::addToBuffer);
}


/*!
 * \brief Sets the port name.
 *
 * The port name is not taken in account. The method exists only because of compatibility with real QSerialPort
 * interface.
 *
 * \param com_port_name The port name.
 */
void MockSerialPort::setPortName(const QString& com_port_name)
{
    Q_UNUSED(com_port_name)
}

/*!
 * \brief Sets baud rate.
 *
 * The default value of baud rate is `QSerialPort::Baud9600`.
 *
 * \param baud_rate The baud rate.
 * \return True if successful.
 */
bool MockSerialPort::setBaudRate(qint32 baud_rate)
{
    baud_rate_ = baud_rate;
    return true;
}


/*!
 * \brief Sets data bits.
 *
 * The default value for data bits is `QSerialPort::Data8`.
 *
 * \param data_bits The data bits.
 * \return True if successful.
 */
bool MockSerialPort::setDataBits(QSerialPort::DataBits data_bits)
{
    data_bits_ = data_bits;
    return true;
}


/*!
 * \brief Sets parity.
 *
 * The default value for parity is `QSerialPort::NoParity`.
 *
 * \param parity The parity
 * \return True if successful.
 */
bool MockSerialPort::setParity(QSerialPort::Parity parity)
{
    parity_ = parity;
    return true;
}


/*!
 * \brief Sets stop bits.
 *
 * The default value for stop bits is `QSerialPort::OneStop`.
 *
 * \param stop_bits The stop bits.
 * \return True if successful.
 */
bool MockSerialPort::setStopBits(QSerialPort::StopBits stop_bits)
{
    stop_bits_ = stop_bits;
    return true;
}


/*!
 * \brief Sets flow control.
 *
 * The default value for flow control is `QSerialPort::NoFlowControl`.
 *
 * \param flow_control The flow control.
 * \return True if successful.
 */
bool MockSerialPort::setFlowControl(QSerialPort::FlowControl flow_control)
{
    flow_control_ = flow_control;
    return true;
}


/*!
 * \brief Tests if the port is open.
 * \return True if open.
 * \sa MockSerialPort::open(), MockSerialPort::close()
 */
bool MockSerialPort::isOpen()
{
    return open_;
}


/*!
 * \brief Opens the port.
 * \param mode Open mode.
 * \return True if successful.
 * \sa MockSerialPort::isOpen(), MockSerialPort::close()
 */
bool MockSerialPort::open(QIODevice::OpenMode mode)
{
    mode_ = mode;
    open_ = true;
    return true;
}


/*!
 * \brief Closes the port.
 * \sa MockSerialPort::open()
 */
void MockSerialPort::close()
{
    open_ = false;
    buffer_.clear();
}


/*!
 * \brief Reads all data in buffer.
 *
 * The communication is asynchronous. When data is ready to read, the MockSerialPort::readyRead() signal is emited.
 * Then you can read the data. Data comes in multiples of 7 bytes. The serial port has to be opened with
 * `QSerialPort::ReadOnly` or `QSerialPort::ReadWrite` mode.
 *
 * \return The data.
 */
QByteArray MockSerialPort::readAll()
{
    if (open_ && mode_ & QIODevice::ReadOnly) {
        return std::move(buffer_);
    } else {
        return QByteArray{};
    }
}


/*!
 * \brief Writes data to serial port.
 *
 * The data should be multiples of 7 bytes. The communication protocol is described in Vellemna %K8090 relay card
 * manual. The serial port has to be opened with `QSerialPort::WriteOnly` or `QSerialPort::ReadWrite` mode.
 *
 * \param data The data.
 * \param max_size The size of data.
 * \return The number of written bytes or -1 in the case of error.
 */
qint64 MockSerialPort::write(const char* data, qint64 max_size)
{
    if (open_ && error_ == QSerialPort::NoError) {
        if (mode_ & QIODevice::WriteOnly && verifyPortParameters()) {
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            sendData(reinterpret_cast<const unsigned char*>(data), max_size);
        }
        return max_size;
    } else {
        return -1;
    }
}


/*!
 * \brief Flushes the buffer.
 *
 * Now, the data writing is treated as synchronous, so it always returns false.
 *
 * \return True if any data was written.
 * \sa MockSerialPort::write()
 */
bool MockSerialPort::flush()
{
    return false;
}


/*!
 * \brief Holds the error status of the serial port.
 * \return The error code.
 * \sa MockSerialPort::clearError()
 */
QSerialPort::SerialPortError MockSerialPort::error()
{
    return error_;
}


/*!
 * \brief Clears error.
 * \sa MockSerialPort::error()
 */
void MockSerialPort::clearError()
{
    error_ = QSerialPort::NoError;
}


/*!
 * \fn MockSerialPort::readyRead()
 * \brief Emited, when some data comes through serial port.
 *
 * The data can be readed with MockSerialPort::readAll() method.
 */


// helper method which moves data from queue with responses to the buffer after timeout of response_timer_. The timer
// is triggered by the methods called from sendData() method which is called from the write() method. The data can be
// added to the buffer only in chunks of 1 - 3 packets, remaining data is added after timeout of response_timer_, which
// is now triggered by this method. After the data is in the buffer, the readyRead() signal is emited. They can be
// read by readAll() method.
void MockSerialPort::addToBuffer()
{
    if (mode_ & QIODevice::ReadOnly) {
        std::uniform_int_distribution<int> distribution{1, 3};
        int max_responses = distribution(random_generator);
        int counter = 0;
        while (!stored_responses_.empty() && counter < max_responses) {
            std::unique_ptr<unsigned char[]> response = std::move(stored_responses_.front());
            stored_responses_.pop();
            // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
            buffer_.append(reinterpret_cast<const char*>(response.get()), 7);
            ++counter;
        }
        if (!stored_responses_.empty()) {
            response_timer_.start(getRandomDelay());
        }
        emit readyRead();
    }
}


// This method is triggered when some relay times out. It checks, if some other relay isn't near to time out and if it
// is it times it out too. This approach effectively merges close relay timeouts to one relay status event emission.
void MockSerialPort::delayTimeout(int i)
{
    unsigned char relay = 1 << i;
    unsigned char relay2;
    // time out also near timers
    for (int j = 0; j < 8; ++j) {
        if (j != i) {
            relay2 = 1 << j;
            if (relay2 & active_timers_ && delay_timers_[j].remainingTime() < kTimerDeltaMs_) {
                relay |= relay2;
                delay_timers_[j].stop();
                active_timers_ &= ~relay2;
            }
        }
    }
    active_timers_ &= ~relay;

    unsigned char previous = on_;
    on_ &= ~relay;
    unsigned char current = on_;

    // insert response to queue with responses
    std::unique_ptr<unsigned char[]> response{new unsigned char[7]{           // wrap
        k8090::impl_::kStxByte,                                               // wrap
        k8090::impl_::kResponses[as_number(k8090::ResponseID::RelayStatus)],  // wrap
        previous,                                                             // wrap
        current,                                                              // wrap
        active_timers_,                                                       // wrap
        0,                                                                    // wrap
        k8090::impl_::kEtxByte}};
    response[5] = k8090::impl_::check_sum(response.get(), 5);
    stored_responses_.push(std::move(response));

    // starts timer to delay response
    if (!response_timer_.isActive()) {
        response_timer_.start(getRandomDelay());
    }
}


// checks port parameters validity
bool MockSerialPort::verifyPortParameters()
{
    if (baud_rate_ != kNeededBaudRate_ || data_bits_ != kNeededDataBits_ || parity_ != kNeededParity_
        || stop_bits_ != kNeedeStopBits_ || flow_control_ != kNeededFlowControl_) {
        return false;
    } else {
        return true;
    }
}


// this method is called from the write() method and decides which command is received
void MockSerialPort::sendData(const unsigned char* buffer, qint64 max_size)
{
    if (max_size < 7) {
        return;
    }
    // TODO(lumik): switch to PIMPL and remove unnecessary heap usage
    std::unique_ptr<k8090::impl_::CardMessage> command;
    try {
        command.reset(new k8090::impl_::CardMessage{buffer, buffer + 7});
    } catch (const std::out_of_range&) {
        return;
    }

    if (!command->isValid()) {
        return;
    }

    switch (buffer[1]) {
        case k8090::impl_::kCommands[as_number(k8090::CommandID::RelayOn)]:
            relayOn(std::move(command));
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::RelayOff)]:
            relayOff(std::move(command));
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::ToggleRelay)]:
            toggleRelay(std::move(command));
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::SetButtonMode)]:
            setButtonMode(std::move(command));
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::ButtonMode)]:
            queryButtonMode();
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::StartTimer)]:
            startRelayTimer(std::move(command));
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::SetTimer)]:
            setRelayTimer(std::move(command));
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::Timer)]:
            queryRelayTimer(std::move(command));
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::QueryRelay)]:
            queryRelay();
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::ResetFactoryDefaults)]:
            factoryDefaults();
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::JumperStatus)]:
            jumperStatus();
            break;
        case k8090::impl_::kCommands[as_number(k8090::CommandID::FirmwareVersion)]:
            firmwareVersion();
            break;
    }
}


// Returns random delya according to binomial distribution. It is used for response delay. See class description.
int MockSerialPort::getRandomDelay()
{
    std::binomial_distribution<int> distribution{
        kMaxResponseDelayMs_ - kMinResponseDelayMs_, kResponseDelayDistributionP};
    return kMinResponseDelayMs_ + distribution(random_generator);
}


// command resolving methods //
// ************************* //
// The methods which have response insert the response in the response queue and start response_timer_, which after
// some delay triggers addToBuffer() method which makes the data available to user in readAll(). Special case is
// timers, which also start timers for corresponding relays. The timers triggers the delayTimeout() method.

// switches specified realys on
void MockSerialPort::relayOn(std::unique_ptr<k8090::impl_::CardMessage> command)
{
    unsigned char previous = on_;
    on_ |= command->data[2];
    unsigned char current = on_;
    if (previous != current) {
        std::unique_ptr<unsigned char[]> response{new unsigned char[7]{k8090::impl_::kStxByte,  // wrap
            k8090::impl_::kResponses[as_number(k8090::ResponseID::RelayStatus)],
            previous,        // wrap
            current,         // wrap
            active_timers_,  // wrap
            0,               // wrap
            k8090::impl_::kEtxByte}};
        response[5] = k8090::impl_::check_sum(response.get(), 5);
        stored_responses_.push(std::move(response));
        if (!response_timer_.isActive()) {
            response_timer_.start(getRandomDelay());
        }
    }
}


// switches specified relays off, stops corresponding timers
void MockSerialPort::relayOff(std::unique_ptr<k8090::impl_::CardMessage> command)
{
    unsigned char previous = on_;
    on_ &= ~command->data[2];
    unsigned char current = on_;
    if (previous != current) {
        for (int i = 0; i < 8; ++i) {
            unsigned char relay = 1 << i;
            if (relay & active_timers_ & command->data[2]) {
                delay_timers_[i].stop();
                active_timers_ &= ~relay;
            }
        }
        std::unique_ptr<unsigned char[]> response{new unsigned char[7]{k8090::impl_::kStxByte,  // wrap
            k8090::impl_::kResponses[as_number(k8090::ResponseID::RelayStatus)],                // wrap
            previous,                                                                           // wrap
            current,                                                                            // wrap
            active_timers_,                                                                     // wrap
            0,                                                                                  // wrap
            k8090::impl_::kEtxByte}};
        response[5] = k8090::impl_::check_sum(response.get(), 5);
        stored_responses_.push(std::move(response));
        if (!response_timer_.isActive()) {
            response_timer_.start(getRandomDelay());
        }
    }
}


// toggles specified relays, stops corresponding timers
void MockSerialPort::toggleRelay(std::unique_ptr<k8090::impl_::CardMessage> command)
{
    unsigned char previous = on_;
    on_ ^= command->data[2];
    unsigned char current = on_;
    if (previous != current) {
        for (int i = 0; i < 8; ++i) {
            unsigned char relay = 1 << i;
            if (relay & active_timers_ & previous & command->data[2]) {
                delay_timers_[i].stop();
                active_timers_ &= ~relay;
            }
        }
        std::unique_ptr<unsigned char[]> response{new unsigned char[7]{k8090::impl_::kStxByte,  // wrap
            k8090::impl_::kResponses[as_number(k8090::ResponseID::RelayStatus)],
            previous,        // wrap
            current,         // wrap
            active_timers_,  // wrap
            0,               // wrap
            k8090::impl_::kEtxByte}};
        response[5] = k8090::impl_::check_sum(response.get(), 5);
        stored_responses_.push(std::move(response));
        if (!response_timer_.isActive()) {
            response_timer_.start(getRandomDelay());
        }
    }
}


// sets button modes
void MockSerialPort::setButtonMode(std::unique_ptr<k8090::impl_::CardMessage> command)
{
    momentary_ = command->data[2];
    toggle_ = command->data[3] & ~momentary_;
    timed_ = command->data[4] & ~(momentary_ | toggle_);
}


// queries button modes
void MockSerialPort::queryButtonMode()
{
    std::unique_ptr<unsigned char[]> response{new unsigned char[7]{k8090::impl_::kStxByte,  // wrap
        k8090::impl_::kResponses[as_number(k8090::ResponseID::ButtonMode)],                 // wrap
        momentary_,                                                                         // wrap
        toggle_,                                                                            // wrap
        timed_,                                                                             // wrap
        0,                                                                                  // wrap
        k8090::impl_::kEtxByte}};
    response[5] = k8090::impl_::check_sum(response.get(), 5);
    stored_responses_.push(std::move(response));
    if (!response_timer_.isActive()) {
        response_timer_.start(getRandomDelay());
    }
}


// starts timers
void MockSerialPort::startRelayTimer(std::unique_ptr<k8090::impl_::CardMessage> command)
{
    int delay_ms = (command->data[3] * 256 + command->data[4]) * 1000;
    int local_delay_ms;  // delay of each timer, changes inside the loop
    unsigned char relay;
    for (int i = 0; i < 8; ++i) {
        relay = 1 << i;
        if (relay & command->data[2]) {
            if (delay_ms) {
                local_delay_ms = delay_ms;
            } else {
                local_delay_ms = default_delays_[i] * 1000;
            }
            delay_timer_delays_[i] = local_delay_ms;
            delay_timers_[i].start(local_delay_ms);
            active_timers_ |= relay;
        }
    }
    unsigned char previous = on_;
    on_ |= command->data[2];
    unsigned char current = on_;
    if (previous != current) {
        std::unique_ptr<unsigned char[]> response{new unsigned char[7]{k8090::impl_::kStxByte,  // wrap
            k8090::impl_::kResponses[as_number(k8090::ResponseID::RelayStatus)],                // wrap
            previous,                                                                           // wrap
            current,                                                                            // wrap
            active_timers_,                                                                     // wrap
            0,                                                                                  // wrap
            k8090::impl_::kEtxByte}};
        response[5] = k8090::impl_::check_sum(response.get(), 5);
        stored_responses_.push(std::move(response));
        if (!response_timer_.isActive()) {
            response_timer_.start(getRandomDelay());
        }
    }
}


// sets default timer timeouts
void MockSerialPort::setRelayTimer(std::unique_ptr<k8090::impl_::CardMessage> command)
{
    k8090::RelayID relay_ids{static_cast<k8090::RelayID>(command->data[2])};
    quint16 delay = 256 * command->data[3] + command->data[4];
    for (unsigned int i = 0; i < 8; ++i) {
        if (as_number(k8090::from_number(i)) & as_number(relay_ids)) {
            default_delays_[i] = delay;
        }
    }
}


// query actual or default timer timeouts
void MockSerialPort::queryRelayTimer(std::unique_ptr<k8090::impl_::CardMessage> command)
{
    k8090::RelayID relay_ids{static_cast<k8090::RelayID>(command->data[2])};
    for (unsigned int i = 0; i < 8; ++i) {
        if (as_number(k8090::from_number(i)) & as_number(relay_ids)) {
            unsigned char high_byte;
            unsigned char low_byte;
            // total timer
            if (!command->data[3]) {
                high_byte = highByte(default_delays_[i]);
                low_byte = lowByte(default_delays_[i]);
            } else {
                quint16 delay;
                if (delay_timers_[i].isActive()) {
                    delay = delay_timers_[i].remainingTime() / 1000 + (delay_timers_[i].remainingTime() % 1000 != 0);
                } else {
                    delay = remaining_delays_[i];
                }
                high_byte = highByte(delay);
                low_byte = lowByte(delay);
            }
            std::unique_ptr<unsigned char[]> response{new unsigned char[7]{// wrap
                k8090::impl_::kStxByte,                                    // wrap
                k8090::impl_::kResponses[as_number(k8090::ResponseID::Timer)],
                as_number(k8090::from_number(i)),  // wrap
                high_byte,                         // wrap
                low_byte,                          // wrap
                0,                                 // wrap
                k8090::impl_::kEtxByte}};
            response[5] = k8090::impl_::check_sum(response.get(), 5);
            stored_responses_.push(std::move(response));
            if (!response_timer_.isActive()) {
                response_timer_.start(getRandomDelay());
            }
        }
    }
}


// queries relay status
void MockSerialPort::queryRelay()
{
    std::unique_ptr<unsigned char[]> response{new unsigned char[7]{           // wrap
        k8090::impl_::kStxByte,                                               // wrap
        k8090::impl_::kResponses[as_number(k8090::ResponseID::RelayStatus)],  // wrap
        on_,                                                                  // wrap
        on_,                                                                  // wrap
        active_timers_,                                                       // wrap
        0,                                                                    // wrap
        k8090::impl_::kEtxByte}};
    response[5] = k8090::impl_::check_sum(response.get(), 5);
    stored_responses_.push(std::move(response));
    if (!response_timer_.isActive()) {
        response_timer_.start(getRandomDelay());
    }
}


// resets to factory defaults
void MockSerialPort::factoryDefaults()
{
    momentary_ = as_number(k8090::RelayID::None);
    toggle_ = as_number(k8090::RelayID::All);
    timed_ = as_number(k8090::RelayID::None);
    for (int i = 0; i < 8; ++i) {
        default_delays_[i] = 5;
    }
    if (on_ != k8090::as_number(k8090::RelayID::None)) {
        // TODO(lumik): switch to PIMPL and remove unnecessary heap usage
        std::unique_ptr<k8090::impl_::CardMessage> off_command{new k8090::impl_::CardMessage{// wrap
            k8090::impl_::kStxByte,                                                          // wrap
            k8090::impl_::kResponses[as_number(k8090::CommandID::RelayOff)],                 // wrap
            on_,                                                                             // wrap
            0,                                                                               // wrap
            0,                                                                               // wrap
            0,                                                                               // wrap
            k8090::impl_::kEtxByte}};
        off_command->checksumMessage();
        relayOff(std::move(off_command));
    }
}


// queries jumper status
void MockSerialPort::jumperStatus()
{
    std::unique_ptr<unsigned char[]> response{new unsigned char[7]{            // wrap
        k8090::impl_::kStxByte,                                                // wrap
        k8090::impl_::kResponses[as_number(k8090::ResponseID::JumperStatus)],  // wrap
        0,                                                                     // wrap
        jumper_status_,                                                        // wrap
        0,                                                                     // wrap
        0,                                                                     // wrap
        k8090::impl_::kEtxByte}};
    response[5] = k8090::impl_::check_sum(response.get(), 5);
    stored_responses_.push(std::move(response));
    if (!response_timer_.isActive()) {
        response_timer_.start(getRandomDelay());
    }
}


// queries firmware version
void MockSerialPort::firmwareVersion()
{
    std::unique_ptr<unsigned char[]> response{new unsigned char[7]{               // wrap
        k8090::impl_::kStxByte,                                                   // wrap
        k8090::impl_::kResponses[as_number(k8090::ResponseID::FirmwareVersion)],  // wrap
        0,                                                                        // wrap
        firmware_version_[0],                                                     // wrap
        firmware_version_[1],                                                     // wrap
        0,                                                                        // wrap
        k8090::impl_::kEtxByte}};
    response[5] = k8090::impl_::check_sum(response.get(), 5);
    stored_responses_.push(std::move(response));
    if (!response_timer_.isActive()) {
        response_timer_.start(getRandomDelay());
    }
}

}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
