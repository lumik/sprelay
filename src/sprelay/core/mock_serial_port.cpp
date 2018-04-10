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
    \file mock_serial_port.cpp
*/

#include "mock_serial_port.h"

namespace sprelay {
namespace core {

/*!
    \class MockSerialPort
    \ingroup Core
    \brief Class which simulates the behavior of relay K8090 card connected through the virtual serial port.

    \remark reentrant
*/

/*!
    \brief Product id for port identification.
*/
const quint16 MockSerialPort::kProductID = 32912;

/*!
    \brief Vendor id for port identification.
*/
const quint16 MockSerialPort::kVendorID = 4303;


/*!
    \brief The constructor.
    \param parent Parent object in Qt ownership system.
*/
MockSerialPort::MockSerialPort(QObject *parent) : QObject{parent}
{
}


/*!
    \brief Sets the port name.
    \param com_port_name The port name.
*/
void MockSerialPort::setPortName(const QString &com_port_name)
{
    Q_UNUSED(com_port_name)
}

/*!
    \brief Sets baud rate.
    \param baud_rate The baud rate.
    \return True if successful.
*/
bool MockSerialPort::setBaudRate(qint32 baud_rate)
{
    Q_UNUSED(baud_rate)
    return true;
}


/*!
    \brief Sets data bits.
    \param data_bits The data bits.
    \return True if successful.
*/
bool MockSerialPort::setDataBits(QSerialPort::DataBits data_bits)
{
    Q_UNUSED(data_bits)
    return true;
}


/*!
    \brief Sets parity.
    \param parity The parity
    \return True if successful.
*/
bool MockSerialPort::setParity(QSerialPort::Parity parity)
{
    Q_UNUSED(parity)
    return true;
}


/*!
    \brief Sets stop bits.
    \param stop_bits The stop bits.
    \return True if successful.
*/
bool MockSerialPort::setStopBits(QSerialPort::StopBits stop_bits)
{
    Q_UNUSED(stop_bits)
    return true;
}


/*!
    \brief Sets flow control.
    \param flow_control The flow control.
    \return True if successful.
*/
bool MockSerialPort::setFlowControl(QSerialPort::FlowControl flow_control)
{
    Q_UNUSED(flow_control)
    return true;
}


/*!
    \brief Tests if the port is open.
    \return True if open.
*/
bool MockSerialPort::isOpen()
{
    return true;
}


/*!
    \brief Opens the port.
    \param mode Open mode.
    \return True if successful.
*/
bool MockSerialPort::open(QIODevice::OpenMode mode)
{
    Q_UNUSED(mode);
    return true;
}


/*!
    \brief Closes the port.
*/
void MockSerialPort::close()
{
}


/*!
    \brief Reads all data in buffer.
    \return The data.

    \sa MockSerialPort::readyRead()
*/
QByteArray MockSerialPort::readAll()
{
    return QByteArray{};
}


/*!
    \brief Writes data to serial port.
    \param data The data.
    \param max_size The size of data.
    \return The number of written bytes or -1 in the case of error.
*/
qint64 MockSerialPort::write(const char *data, qint64 max_size)
{
    Q_UNUSED(data)
    Q_UNUSED(max_size)
    return 0;
}


/*!
    \brief Flushes the buffer.
    \return True if any data was written.
    \sa MockSerialPort::write()
*/
bool MockSerialPort::flush()
{
    return true;
}


/*!
    \brief Holds the error status of the serial port.
    \return The error code.
    \sa MockSerialPort::clearError()
*/
QSerialPort::SerialPortError MockSerialPort::error()
{
    return QSerialPort::NoError;
}


/*!
    \brief Clears error.
    \sa MockSerialPort::error()
*/
void MockSerialPort::clearError()
{
}


/*!
    \fn MockSerialPort::readyRead()
    \brief Emited, when some data comes through serial port.

    The data can be readed with MockSerialPort::readAll() method.
*/

}  // namespace core
}  // namespace sprelay
