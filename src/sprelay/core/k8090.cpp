/***************************************************************************
**                                                                        **
**  Controlling interface for K8090 8-Channel Relay Card from Velleman    **
**  through usb using virtual serial port in Qt.                          **
**  Copyright (C) 2017 Jakub Klener                                       **
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
    \file k8090.h
*/

#include "k8090.h"

#include <QDebug>
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QStringBuilder>

/*!
    \namespace K8090Traits
    \brief Contains traits for K8090 class.
*/
using namespace K8090Traits;  // NOLINT(build/namespaces)

/*!
    \enum Command
    \brief Scoped enumeration listing all commands.

    See the Velleman %K8090 card manual.
*/
/*!
    \var Command::RELAY_ON
    \brief Switch realy on command.
*/
/*!
    \var Command::RELAY_OFF
    \brief Switch realy off command.
*/
/*!
    \var Command::TOGGLE_RELAY
    \brief Toggle realy command.
*/
/*!
    \var Command::QUERY_RELAY
    \brief Query relay status command.
*/
/*!
    \var Command::SET_BUTTON_MODE
    \brief Set button mode command.
*/
/*!
    \var Command::BUTTON_MODE
    \brief Query button mode command.
*/
/*!
    \var Command::START_TIMER
    \brief Start relay timer command.
*/
/*!
    \var Command::SET_TIMER
    \brief Set relay timer delay command.
*/
/*!
    \var Command::TIMER
    \brief Query timer delay command.
*/
/*!
    \var Command::BUTTON_STATUS
    \brief Button status command.
*/
/*!
    \var Command::RELAY_STATUS
    \brief Relay status command.
*/
/*!
    \var Command::RESET_FACTORY_DEFAULTS
    \brief Reset factory defaults command.
*/
/*!
    \var Command::JUMPER_STATUS
    \brief Jumper status command.
*/
/*!
    \var Command::FIRMWARE_VERSION
    \brief Firmware version command.
*/
/*!
    \var Command::NONE
    \brief The number of all commands represents also none command.
*/

/*!
    \enum RelayID
    \brief Scoped enumeration listing all 8 relays.

    Bitwise operators are enabled for this enum by overloading K8090Traits::enableBitmaskOperators(RelayID) function
    (see enum_flags.h for more details) and so the value of K8090Traits::RelayID type can be also a combination of
    particular relays.
*/
/*!
    \var RelayID::NONE
    \brief None relay.
*/
/*!
    \var RelayID::ONE
    \brief First relay.
*/
/*!
    \var RelayID::TWO
    \brief Second relay.
*/
/*!
    \var RelayID::THREE
    \brief Third relay.
*/
/*!
    \var RelayID::FOUR
    \brief Fourth relay.
*/
/*!
    \var RelayID::FIVE
    \brief Fifth relay.
*/
/*!
    \var RelayID::SIX
    \brief Sixth relay.
*/
/*!
    \var RelayID::SEVEN
    \brief Seventh relay.
*/
/*!
    \var RelayID::EIGHT
    \brief Eigth relay.
*/
/*!
    \var RelayID::ALL
    \brief All relays.
*/

/*!
    \fn constexpr bool enableBitmaskOperators(RelayID)
    \brief Function overload which enables bitwise operators for RelayID enumeration. See enum_flags.h for more
    details.

    \return True to enable bitmask operators.
*/

/*!
    \fn constexpr std::underlying_type<E> as_number(const E e)
    \brief Converts enumeration to its underlying type.

    \param e Enumerator to be converted.
    \return The enum value as underlying type.
*/

// static constants
const quint16 K8090::kProductID = 32912;
const quint16 K8090::kVendorID = 4303;

/*!
    \brief Start delimiting byte.
*/
const unsigned char kStxByte_ = 0x04;
/*!
    \brief End delimiting byte.
*/
const unsigned char kEtxByte_ = 0x0f;

// generate static array containing commands at compile time

namespace {  // unnamed namespace

// template function to fill the array with appropriate commands
template<unsigned int N>
constexpr unsigned char getXDataValue();

// specializations
template<>
constexpr unsigned char getXDataValue<as_number(Command::RELAY_ON)>()
{
    return 0x11;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::RELAY_OFF)>()
{
    return 0x12;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::TOGGLE_RELAY)>()
{
    return 0x14;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::QUERY_RELAY)>()
{
    return 0x18;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::SET_BUTTON_MODE)>()
{
    return 0x21;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::BUTTON_MODE)>()
{
    return 0x22;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::START_TIMER)>()
{
    return 0x41;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::SET_TIMER)>()
{
    return 0x42;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::TIMER)>()
{
    return 0x44;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::BUTTON_STATUS)>()
{
    return 0x50;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::RELAY_STATUS)>()
{
    return 0x51;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::RESET_FACTORY_DEFAULTS)>()
{
    return 0x66;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::JUMPER_STATUS)>()
{
    return 0x70;
}
template<>
constexpr unsigned char getXDataValue<as_number(Command::FIRMWARE_VERSION)>()
{
    return 0x71;
}


// Template containing static array
template<unsigned char ...Args>
struct XArrayData
{
    static const unsigned char kValues[sizeof...(Args)];
};

// recursively generates XData typedef
template<unsigned int N, unsigned char ...Args>
struct XArrayGenerator_
{
    typedef typename XArrayGenerator_<N - 1, getXDataValue<N - 1>(), Args...>::XData XData;
};

// end case template partial specialization
template<unsigned char ...Args>
struct XArrayGenerator_<1u, Args...>
{
    typedef XArrayData<getXDataValue<0u>(), Args...> XData;
};

// XArray generates recursively XData type, which contains static constant array kValues.
// Usage: unsigned char arr = XArray<K8090Traits::Comand::None>::XData::kValues
template<unsigned char N>
struct XArray
{
    typedef typename XArrayGenerator_<N>::XData XData;
};

// static const array initialization
template<unsigned char ...Args>
const unsigned char XArrayData<Args...>::kValues[sizeof...(Args)] = {Args...};

}  // unnamed namespace


/*!
    \class K8090
    \brief The class that provides the interface for Velleman %K8090 relay card
    controlling through serial port.

    \remark reentrant, thread-safe
*/

// initialization of static member variables
/*!
    \brief Array of hexadecimal representation of commands used to control the relay.

    They should be accessed using the K8090Traits::Command enum values.

    Example, which shows how to build the whole command read status:

    \code
    int n = 7;  // Number of command bytes.
    unsigned char cmd[n]; // array of command bytes

    // copying the first two bytes of command to command byte array
    cmd[0] = kStxByte_
    cmd[1] = commands_[as_number(Command::RELAY_STATUS)];
    cmd[2] = as_number(RelayID::ONE);  // 3rd byte specifies affected relays
    // commands, there is no one.
    cmd[5] = K8090::checkSum(cmd, 5); // sixth byte contains check sum.
    cmd[6] = kEtxByte_;
    \endcode
*/
const unsigned char *K8090::commands_ = XArray<as_number(Command::NONE)>::XData::kValues;


/*!
  \brief Creates a new K8090 instance and sets the default values.
*/
K8090::K8090(QObject *parent) :
    QObject(parent)
{
    last_command_ = Command::NONE;

    serial_port_ = new QSerialPort(this);
    connect(serial_port_, &QSerialPort::readyRead, this, &K8090::onReadyData);
}


QList<ComPortParams> K8090::availablePorts()
{
    QList<ComPortParams> comPortParamsList;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {  // NOLINT(whitespace/parens)
        ComPortParams comPortParams;
        comPortParams.portName = info.portName();
        comPortParams.description = info.description();
        comPortParams.manufacturer = info.manufacturer();
        comPortParams.productIdentifier = info.productIdentifier();
        comPortParams.vendorIdentifier = info.vendorIdentifier();
        comPortParamsList.append(comPortParams);
    }
    return comPortParamsList;
}


void K8090::connectK8090()
{
    bool cardFound = false;
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts()) {  // NOLINT(whitespace/parens)
        if (info.productIdentifier() == kProductID &&
                info.vendorIdentifier() == kVendorID) {
            cardFound = true;
            com_port_name_ = info.portName();
            qDebug() << "Port name: " % com_port_name_;
        }
    }

    if (!cardFound) {
        qDebug() << "Card not found!!!";
        return;
    }

    serial_port_->setPortName(com_port_name_);
    serial_port_->setBaudRate(QSerialPort::Baud19200);
    serial_port_->setDataBits(QSerialPort::Data8);
    serial_port_->setParity(QSerialPort::NoParity);
    serial_port_->setStopBits(QSerialPort::OneStop);
    serial_port_->setFlowControl(QSerialPort::NoFlowControl);

    sendSwitchRelayOffCommand();
}


/*!
   \brief K8090::onSendToSerial
   \param buffer
   \param n
 */
void K8090::onSendToSerial(const unsigned char *buffer, int n)
{
    qDebug() << byteToHex(buffer, n);
    if (!serial_port_->isOpen())
        serial_port_->open(QIODevice::ReadWrite);
    serial_port_->write(reinterpret_cast<char*>(const_cast<unsigned char*>(buffer)), n);
    delete[] buffer;
}


void K8090::onReadyData()
{
    qDebug() << "R8090::onReadyData";

    // converting the data to unsigned char
    QByteArray data = serial_port_->readAll();
    int n = data.size();
    unsigned char *buffer = reinterpret_cast<unsigned char*>(data.data());
    qDebug() << byteToHex(buffer, n);

    last_command_ = Command::NONE;
}


/*!
    \brief K8090::sendCommand
*/
void K8090::sendCommand()
{
}


/*!
   \brief K8090::sendSwitchRelayOnCommand
 */
void K8090::sendSwitchRelayOnCommand()
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    cmd[0] = kStxByte_;
    cmd[1] = commands_[as_number(Command::RELAY_ON)];
    cmd[2] = (unsigned char) RelayID::ONE;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = kEtxByte_;
    last_command_ = Command::RELAY_ON;
    qDebug() << byteToHex(cmd, n);
    onSendToSerial(cmd, n);
}


/*!
   \brief K8090::sendSwitchRelayOnCommand
 */
void K8090::sendSwitchRelayOffCommand()
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    cmd[0] = kStxByte_;
    cmd[1] = commands_[as_number(Command::RELAY_OFF)];
    cmd[2] = (unsigned char) RelayID::ONE;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = kEtxByte_;
    last_command_ = Command::RELAY_OFF;
    qDebug() << byteToHex(cmd, n);
    onSendToSerial(cmd, n);
}


/*!
   \brief K8090::hexToByte
   \param pbuffer
   \param n
   \param msg
 */
void K8090::hexToByte(unsigned char **pbuffer, int *n, const QString &msg)
{
    // remove white spaces
    QString newMsg = msg;
    newMsg.remove(' ');

    int msgSize = newMsg.size();

    // test correct size of msg, all hex codes consit of 2 characters
    if (msgSize % 2) {
        *pbuffer = nullptr;
        *n = 0;
    } else {
        *n = msgSize / 2;
        *pbuffer = new unsigned char[*n];
        bool ok;
        for (int ii = 0; ii < *n; ++ii) {
            (*pbuffer)[ii] = newMsg.midRef(2 * ii, 2).toUInt(&ok, 16);
        }
    }
}


/*!
   \brief K8090::byteToHex
   \param buffer
   \param n
   \return
 */
QString K8090::byteToHex(const unsigned char *buffer, int n)
{
    QString msg;
    for (int ii = 0; ii < n - 1; ++ii) {
        msg.append(QString("%1").arg((unsigned int)buffer[ii], 2, 16, QChar('0')).toUpper()).append(' ');
    }
    if (n > 0) {
        msg.append(QString("%1").arg((unsigned int)buffer[n - 1], 2, 16, QChar('0')).toUpper());
    }
    return msg;
}


/*!
   \brief K8090::checkSum
   \param msg
   \return
 */
QString K8090::checkSum(const QString &msg)
{
    unsigned char *bMsg;
    int n;
    hexToByte(&bMsg, &n, msg);

    unsigned char bChk = checkSum(bMsg, n);

    delete[] bMsg;

    return QString("%1").arg((unsigned int)bChk, 2, 16, QChar('0')).toUpper();
}


/*!
   \brief K8090::checkSum
   \param bMsg
   \param n
   \return
 */
unsigned char K8090::checkSum(const unsigned char *bMsg, int n)
{
    unsigned int iSum = 0u;
    for (int ii = 0; ii < n; ++ii) {
        iSum += (unsigned int)bMsg[ii];
    }
    unsigned char byteSum = iSum % 256;
    iSum = (unsigned int) (~byteSum) + 1u;
    byteSum = (unsigned char) iSum % 256;

    return byteSum;
}


/*!
   \brief K8090::validateResponse
   \param msg
   \param cmd
   \return
 */
bool K8090::validateResponse(const QString &msg, Command cmd)
{
    unsigned char *bMsg;
    int n;
    hexToByte(&bMsg, &n, msg);
    if (validateResponse(bMsg, n, cmd)) {
        delete[] bMsg;
        return true;
    } else {
        delete[] bMsg;
        return false;
    }
}


/*!
    \brief K8090::validateResponse
    \param bMsg Pointer to field of bytes containing the response.
    \param n    The number of command bytes.
    \param cmd  The last command enum value.
    \return     true if response is valid, false if not.
*/
bool K8090::validateResponse(const unsigned char *bMsg, int n, Command cmd)
{
    if (n >= 6) {
        if (bMsg[0] != kStxByte_)
            return false;
        if (bMsg[1] != commands_[as_number(cmd)])
            return false;
        unsigned char bTest[5];
        for (int ii = 0; ii < 5; ++ii)
            bTest[ii] = bMsg[ii];
        unsigned char bChkSum = checkSum(bTest, 5);
        if ((unsigned int)bChkSum == (unsigned int)bMsg[5])
            return true;
    }
    return false;
}


/*!
   \brief K8090::~K8090
 */
K8090::~K8090()
{
    serial_port_->close();
    delete serial_port_;
}
