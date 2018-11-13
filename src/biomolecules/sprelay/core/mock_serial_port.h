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
 * \file      mock_serial_port.h
 * \brief     The biomolecules::sprelay::core::MockSerialPort class which implements the simulated relay card.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_CORE_MOCK_SERIAL_PORT_H_
#define BIOMOLECULES_SPRELAY_CORE_MOCK_SERIAL_PORT_H_

#include <memory>
#include <queue>
#include <vector>

#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QSerialPort>
#include <QSignalMapper>
#include <QString>
#include <QTimer>


namespace biomolecules {
namespace sprelay {
namespace core {

// forward declarations
namespace k8090 {
namespace impl_ {
struct CardMessage;
}  // namespace impl_
}  // namespace k8090

/// \brief Class which simulates the behavior of Velleman relay %K8090 card connected through the virtual serial port.
/// \headerfile ""
class MockSerialPort : public QObject
{
    Q_OBJECT
public:
    static const quint16 kProductID;
    static const quint16 kVendorID;

    explicit MockSerialPort(QObject* parent = nullptr);

    void setPortName(const QString& com_port_name);
    bool setBaudRate(qint32 baud_rate);
    bool setDataBits(QSerialPort::DataBits data_bits);
    bool setParity(QSerialPort::Parity parity);
    bool setStopBits(QSerialPort::StopBits stop_bits);
    bool setFlowControl(QSerialPort::FlowControl flow_control);

    bool isOpen();
    bool open(QIODevice::OpenMode mode);
    void close();

    QByteArray readAll();
    qint64 write(const char* data, qint64 max_size);
    bool flush();

    QSerialPort::SerialPortError error();
    void clearError();

signals:
    void readyRead();

private slots:
    void addToBuffer();
    void delayTimeout(int i);

private:
    static const int kMinResponseDelayMs_;
    static const int kMaxResponseDelayMs_;
    static const float kResponseDelayDistributionP;

    static const qint32 kNeededBaudRate_;
    static const QSerialPort::DataBits kNeededDataBits_;
    static const QSerialPort::Parity kNeededParity_;
    static const QSerialPort::StopBits kNeedeStopBits_;
    static const QSerialPort::FlowControl kNeededFlowControl_;

    static const int kTimerDeltaMs_;  // interval for which the timer is treated as if started at the same time

    bool verifyPortParameters();
    void sendData(const unsigned char* buffer, qint64 max_size);
    static inline unsigned char lowByte(quint16 delay) { return delay & 0xFFu; }
    static inline unsigned char highByte(quint16 delay) { return static_cast<quint16>(delay >> 8u) & 0xFFu; }
    static int getRandomDelay();

    void relayOn(std::unique_ptr<k8090::impl_::CardMessage> command);
    void relayOff(std::unique_ptr<k8090::impl_::CardMessage> command);
    void toggleRelay(std::unique_ptr<k8090::impl_::CardMessage> command);
    void setButtonMode(std::unique_ptr<k8090::impl_::CardMessage> command);
    void queryButtonMode();
    void startRelayTimer(std::unique_ptr<k8090::impl_::CardMessage> command);
    void setRelayTimer(std::unique_ptr<k8090::impl_::CardMessage> command);
    void queryRelayTimer(std::unique_ptr<k8090::impl_::CardMessage> command);
    void queryRelay();
    void factoryDefaults();
    void jumperStatus();
    void firmwareVersion();

    qint32 baud_rate_;
    QSerialPort::DataBits data_bits_;
    QSerialPort::Parity parity_;
    QSerialPort::StopBits stop_bits_;
    QSerialPort::FlowControl flow_control_;
    QSerialPort::SerialPortError error_;

    bool open_;
    QIODevice::OpenMode mode_;
    unsigned char on_;
    unsigned char momentary_;
    unsigned char toggle_;
    unsigned char timed_;
    unsigned char pressed_;
    qint16 default_delays_[8];
    qint16 remaining_delays_[8];  // default value for remaining delay if the timer is not running
    QTimer delay_timers_[8];
    int delay_timer_delays_[8];  // delay, with which the timer was started
    unsigned char active_timers_;
    unsigned char jumper_status_;
    unsigned char firmware_version_[2];

    std::unique_ptr<QSignalMapper> delay_timer_mapper_;
    std::queue<std::unique_ptr<unsigned char[]>> stored_responses_;
    QByteArray buffer_;
    QTimer response_timer_;
};

}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPRELAY_CORE_MOCK_SERIAL_PORT_H_
