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

#ifndef SPRELAY_CORE_MOCK_SERIAL_PORT_H_
#define SPRELAY_CORE_MOCK_SERIAL_PORT_H_

#include <QByteArray>
#include <QIODevice>
#include <QObject>
#include <QSerialPort>
#include <QString>

namespace sprelay {
namespace core {

class MockSerialPort : public QObject
{
    Q_OBJECT
public:  // NOLINT(whitespace/indent)
    static const quint16 kProductID;
    static const quint16 kVendorID;

    explicit MockSerialPort(QObject *parent = nullptr);

    void setPortName(const QString &com_port_name);
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

signals:  // NOLINT(whitespace/indent)
    void readyRead();

private:  // NOLINT(whitespace/indent)
};

}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_MOCK_SERIAL_PORT_H_
