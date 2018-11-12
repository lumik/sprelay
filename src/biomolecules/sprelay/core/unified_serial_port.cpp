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
 * \file      unified_serial_port.cpp
 * \brief     The biomolecules::sprelay::core::UnifiedSerialPort class which combines together real and simulated
 *            connection to %K8090 relay card.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-04-10
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "unified_serial_port.h"

#include <QMutex>
#include <QSerialPort>
#include <QSerialPortInfo>

#include "k8090_commands.h"
#include "mock_serial_port.h"

namespace biomolecules {
namespace sprelay {
namespace core {


/*!
 * \class UnifiedSerialPort
 * The port can be switched by port name. The mock serial port is used as port with name
 * UnifiedSerialPort::kMockPortName. The mock port is added to the list of available serial ports which can be
 * obtained by the UnifiedSerialPort::availablePorts() method.
 *
 * Parameters, as port name, baud rate, data bits, parity, stop bits and flow control are set to port only if some
 * port is openned. Otherwise, they are stored only for later use when the port is opened. When the port is switched
 * from real to mock or other way, the parameters are moved to the new one. More information about the methods can be
 * obtained from QSerialPort documentation.
 *
 * \remark reentrant, thread-safe
 */


/*!
 * \brief The name of mock com port.
 */
const char* UnifiedSerialPort::kMockPortName = k8090::impl_::kMockPortName;


/*!
 * \brief Returns a list of available serial ports extended with mock serial port.
 * \return The ports list.
 * \remark reentrant, thread-safe
 */
QList<serial_utils::ComPortParams> UnifiedSerialPort::availablePorts()
{
    QList<serial_utils::ComPortParams> com_port_params_list;
    // QSerialPortInfo::availablePorts() returns QList by value so it should be thread safe. Implicit sharing should
    // not be a problem because it makes copy on write.
    for (const QSerialPortInfo& info : QSerialPortInfo::availablePorts()) {
        serial_utils::ComPortParams com_port_params;
        com_port_params.port_name = info.portName();
        com_port_params.description = info.description();
        com_port_params.manufacturer = info.manufacturer();
        com_port_params.product_identifier = info.productIdentifier();
        com_port_params.vendor_identifier = info.vendorIdentifier();
        com_port_params_list.append(com_port_params);
    }
    // add mock port
    serial_utils::ComPortParams com_port_params;
    com_port_params.port_name = kMockPortName;
    com_port_params.description = "Mock K8090 card serial port.";
    com_port_params.manufacturer = "Sprelay";
    com_port_params.product_identifier = MockSerialPort::kProductID;
    com_port_params.vendor_identifier = MockSerialPort::kVendorID;
    com_port_params_list.append(com_port_params);
    return com_port_params_list;
}


/*!
 * \brief Constructor.
 * \param parent Parent object in Qt ownership system.
 */
UnifiedSerialPort::UnifiedSerialPort(QObject* parent)
    : QObject{parent},
      serial_port_mutex_{new QMutex},
      port_name_pristine_{true},
      baud_rate_pristine_{true},
      data_bits_pristine_{true},
      parity_pristine_{true},
      stop_bits_pristine_{true},
      flow_control_pristine_{true}
{}


/*!
 * \brief Destructor.
 *
 * Defined to enable forward declarations.
 */
UnifiedSerialPort::~UnifiedSerialPort() {}


/*!
 * \brief Sets port name.
 * \param port_name The port name.
 */
void UnifiedSerialPort::setPortName(const QString& port_name)
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    port_name_ = port_name;
    port_name_pristine_ = false;
    if (isRealImpl()) {
        serial_port_->setPortName(port_name);
    } else if (isMockImpl()) {
        mock_serial_port_->setPortName(port_name);
    }
}


/*!
 * \brief Sets baud rate.
 * \param baud_rate The baud rate.
 * \return True if successful.
 */
bool UnifiedSerialPort::setBaudRate(qint32 baud_rate)
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    baud_rate_ = baud_rate;
    baud_rate_pristine_ = false;
    if (isRealImpl()) {
        return serial_port_->setBaudRate(baud_rate);
    } else if (isMockImpl()) {
        return mock_serial_port_->setBaudRate(baud_rate);
    } else {
        return true;
    }
}


/*!
 * \brief Sets data bits.
 * \param data_bits The data bits.
 * \return True if successful.
 */
bool UnifiedSerialPort::setDataBits(QSerialPort::DataBits data_bits)
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    data_bits_ = data_bits;
    data_bits_pristine_ = false;
    if (isRealImpl()) {
        return serial_port_->setDataBits(data_bits);
    } else if (isMockImpl()) {
        return mock_serial_port_->setDataBits(data_bits);
    } else {
        return true;
    }
}


/*!
 * \brief Sets parity.
 * \param parity The parity
 * \return True if successful.
 */
bool UnifiedSerialPort::setParity(QSerialPort::Parity parity)
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    parity_ = parity;
    parity_pristine_ = false;
    if (isRealImpl()) {
        return serial_port_->setParity(parity);
    } else if (isMockImpl()) {
        return mock_serial_port_->setParity(parity);
    } else {
        return true;
    }
}


/*!
 * \brief Sets stop bits.
 * \param stop_bits The stop bits.
 * \return True if successful.
 */
bool UnifiedSerialPort::setStopBits(QSerialPort::StopBits stop_bits)
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    stop_bits_ = stop_bits;
    stop_bits_pristine_ = false;
    if (isRealImpl()) {
        return serial_port_->setStopBits(stop_bits);
    } else if (isMockImpl()) {
        return mock_serial_port_->setStopBits(stop_bits);
    } else {
        return true;
    }
}


/*!
 * \brief Sets flow control.
 * \param flow_control The flow control.
 * \return True if successful.
 */
bool UnifiedSerialPort::setFlowControl(QSerialPort::FlowControl flow_control)
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    flow_control_ = flow_control;
    flow_control_pristine_ = false;
    if (isRealImpl()) {
        return serial_port_->setFlowControl(flow_control);
    } else if (isMockImpl()) {
        return mock_serial_port_->setFlowControl(flow_control);
    } else {
        return true;
    }
}


/*!
 * \brief Tests if the port is open.
 * \return True if open.
 */
bool UnifiedSerialPort::isOpen()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (isRealImpl()) {
        return serial_port_->isOpen();
    } else if (isMockImpl()) {
        return mock_serial_port_->isOpen();
    } else {
        return false;
    }
}


/*!
 * \brief Opens the port.
 *
 * Port type selection between real and mock serial port is done according to the port name.
 *
 * \param mode Open mode.
 * \return True if successful.
 * \sa UnifiedSerialPort::setPortName()
 */
bool UnifiedSerialPort::open(QIODevice::OpenMode mode)
{
    // until now, serial port has been connected and its name changes
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (serial_port_) {
        // change to mock serial port
        if (!port_name_pristine_ && port_name_ == kMockPortName) {
            if (!createMockPort()) {
                return false;
            }
            return mock_serial_port_->open(mode);
        } else {
            // change only port name
            return serial_port_->open(mode);
        }
    } else {
        // serial port is not connected
        // changing to mock serial, !!! mock port can't have pristine port name because the only way to set mock port
        // is by name
        if (!port_name_pristine_ && port_name_ == kMockPortName) {
            if (!mock_serial_port_ && !createMockPort()) {
                return false;
            }
            return mock_serial_port_->open(mode);
        } else {
            // creating new serial port
            if (!createSerialPort()) {
                return false;
            }
            return serial_port_->open(mode);
        }
    }
}


/*!
 * \brief Closes the port.
 */
void UnifiedSerialPort::close()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (isRealImpl()) {
        serial_port_->close();
    } else if (isMockImpl()) {
        mock_serial_port_->close();
    }
}


/*!
 * \brief Reads all data in buffer.
 * \return The data.
 *
 * \sa UnifiedSerialPort::readyRead()
 */
QByteArray UnifiedSerialPort::readAll()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (isRealImpl()) {
        return serial_port_->readAll();
    } else if (isMockImpl()) {
        return mock_serial_port_->readAll();
    } else {
        return QByteArray{};
    }
}


/*!
 * \brief Writes data to serial port.
 * \param data The data.
 * \param max_size The size of data.
 * \return The number of written bytes or -1 in the case of error.
 */
qint64 UnifiedSerialPort::write(const char* data, qint64 max_size)
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (isRealImpl()) {
        return serial_port_->write(data, max_size);
    } else if (isMockImpl()) {
        return mock_serial_port_->write(data, max_size);
    } else {
        return -1;
    }
}


/*!
 * \brief Nonblockingly flushes the buffer.
 * \return True if any data was written.
 * \sa UnifiedSerialPort::write(), UnifiedSerialPort::waitForBytesWritten()
 */
bool UnifiedSerialPort::flush()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (isRealImpl()) {
        return serial_port_->flush();
    } else if (isMockImpl()) {
        return mock_serial_port_->flush();
    } else {
        return false;
    }
}

/*!
 * \brief Holds the error status of the serial port.
 * \return The error code.
 * \sa UnifiedSerialPort::clearError()
 */
QSerialPort::SerialPortError UnifiedSerialPort::error()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (isRealImpl()) {
        return serial_port_->error();
    } else if (isMockImpl()) {
        return mock_serial_port_->error();
    }
    return QSerialPort::NoError;
}


/*!
 * \brief Clears error.
 * \sa UnifiedSerialPort::error()
 */
void UnifiedSerialPort::clearError()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    if (isRealImpl()) {
        return serial_port_->clearError();
    } else {
        return mock_serial_port_->clearError();
    }
}


/*!
 * \brief Tests if the serial port is mock now.
 *
 * The port can be mock only when some port is created with UnifiedSerialPort::open() method.
 *
 * \return True if mock.
 * \sa UnifiedSerialPort::isReal()
 */
bool UnifiedSerialPort::isMock()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    return isMockImpl();
}


/*!
 * \brief Tests if the serial port is real now.
 *
 * The port can be real only when some port is created with UnifiedSerialPort::open() method.
 *
 * \return True if real.
 * \sa UnifiedSerialPort::isMock()
 */
bool UnifiedSerialPort::isReal()
{
    QMutexLocker serial_port_locker{serial_port_mutex_.get()};
    return isRealImpl();
}


/*!
 * \fn UnifiedSerialPort::readyRead()
 * \brief Emited, when some data comes through serial port.
 *
 * The data can be readed with UnifiedSerialPort::readAll() method.
 */


// isMock() mplementation
bool UnifiedSerialPort::isMockImpl()
{
    return mock_serial_port_ != nullptr;
}


// isReal() implementation
bool UnifiedSerialPort::isRealImpl()
{
    return serial_port_ != nullptr;
}


// helper method which resets private variables to represent real serial port
// !!! Beare, it is not threa-safe, you have to treat thread-safety externaly !!!
bool UnifiedSerialPort::createSerialPort()
{
    serial_port_.reset(new QSerialPort);
    mock_serial_port_.reset();
    connect(serial_port_.get(), &QSerialPort::readyRead, this, &UnifiedSerialPort::readyRead);
    return setupPort(serial_port_.get());
}


// helper method which resets private variables to represent mock serial port
// !!! Beare, it is not threa-safe, you have to treat thread-safety externaly !!!
bool UnifiedSerialPort::createMockPort()
{
    mock_serial_port_.reset(new MockSerialPort);
    serial_port_.reset();
    connect(mock_serial_port_.get(), &MockSerialPort::readyRead, this, &UnifiedSerialPort::readyRead);
    return setupPort(mock_serial_port_.get());
}


// moves port parameters to the newly created one
// !!! Beare, it is not threa-safe, you have to treat thread-safety externaly !!!
template<typename TSerialPort>
bool UnifiedSerialPort::setupPort(TSerialPort* serial_port)
{
    // TODO(lumik): change return code to exception
    if (!port_name_pristine_) {
        serial_port->setPortName(port_name_);
    }
    if (!baud_rate_pristine_) {
        if (!serial_port->setBaudRate(baud_rate_)) {
            return false;
        }
    }
    if (!data_bits_pristine_) {
        if (!serial_port->setDataBits(data_bits_)) {
            return false;
        }
    }
    if (!parity_pristine_) {
        if (!serial_port->setParity(parity_)) {
            return false;
        }
    }
    if (!stop_bits_pristine_) {
        if (!serial_port->setStopBits(stop_bits_)) {
            return false;
        }
    }
    if (!flow_control_pristine_) {
        if (!serial_port->setFlowControl(flow_control_)) {
            return false;
        }
    }
    return true;
}

}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules
