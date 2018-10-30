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
 * \file      unified_serial_port.h
 * \brief     The sprelay::core::UnifiedSerialPort class which combines together real and simulated connection to
 *            %K8090 relay card.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_CORE_UNIFIED_SERIAL_PORT_H_
#define SPRELAY_CORE_UNIFIED_SERIAL_PORT_H_

#include <memory>

#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QSerialPort>
#include <QString>

#include "serial_port_defines.h"
#include "serial_port_utils.h"


// forward declarations
class QMutex;


namespace sprelay {
namespace core {

// forward declarations
class MockSerialPort;


/// \brief Class which unifies QSerialPort and sprelay::core::MockSerialPort and can internaly switch between them.
/// \headerfile ""
class UnifiedSerialPort : public QObject
{
    Q_OBJECT

public:  // NOLINT(whitespace/indent)
    static const char *kMockPortName;

    static QList<serial_utils::ComPortParams> availablePorts();

    explicit UnifiedSerialPort(QObject *parent = nullptr);
    ~UnifiedSerialPort() override;

    void setPortName(const QString &port_name);
    bool setBaudRate(qint32 baud_rate);
    bool setDataBits(QSerialPort::DataBits data_bits);
    bool setParity(QSerialPort::Parity parity);
    bool setStopBits(QSerialPort::StopBits stop_bits);
    bool setFlowControl(QSerialPort::FlowControl flow_control);

    bool isOpen();
    bool open(QIODevice::OpenMode mode);
    void close();

    QByteArray readAll();
    qint64 write(const char *data, qint64 max_size);
    bool flush();

    QSerialPort::SerialPortError error();
    void clearError();

    bool isMock();
    bool isReal();

signals:  // NOLINT(whitespace/indent)
    void readyRead();

private:  // NOLINT(whitespace/indent)
    bool isMockImpl();
    bool isRealImpl();
    bool createSerialPort();
    bool createMockPort();
    template<typename TSerialPort>
    bool setupPort(TSerialPort *serial_port);

    std::unique_ptr<QSerialPort> serial_port_;
    std::unique_ptr<MockSerialPort, serial_utils::MockSerialPortDeleter> mock_serial_port_;
    std::unique_ptr<QMutex> serial_port_mutex_;
    QString port_name_;
    bool port_name_pristine_;
    qint32 baud_rate_;
    bool baud_rate_pristine_;
    QSerialPort::DataBits data_bits_;
    bool data_bits_pristine_;
    QSerialPort::Parity parity_;
    bool parity_pristine_;
    QSerialPort::StopBits stop_bits_;
    bool stop_bits_pristine_;
    QSerialPort::FlowControl flow_control_;
    bool flow_control_pristine_;
};

}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_UNIFIED_SERIAL_PORT_H_
