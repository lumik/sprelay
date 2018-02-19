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
    \brief Contains K8090 traits
*/
using namespace K8090Traits;  // NOLINT(build/namespaces)

// static constants
const quint16 K8090::kProductID = 32912;
const quint16 K8090::kVendorID = 4303;

const QString K8090::kStxByte_ = "04";
const QString K8090::kEtxByte_ = "0F";
const QString K8090::kSwitchRelayOnCmd_ = "11";
const QString K8090::kSwitchRelayOffCmd_ = "12";
const QString K8090::kToggleRelayCmd_ = "14";
const QString K8090::kQueryRelayStatusCmd_ = "18";
const QString K8090::kSetButtonModeCmd_ = "21";
const QString K8090::kQueryButtonModeCmd_ = "22";
const QString K8090::kStartRelayTimerCmd_ = "41";
const QString K8090::kSetRelayTimerDelayCmd_ = "42";
const QString K8090::kQueryTimerDelayCmd_ = "44";
const QString K8090::kButtonStatusCmd_ = "50";
const QString K8090::kRelayStatusCmd_ = "51";
const QString K8090::kResetFactoryDefaultsCmd_ = "66";
const QString K8090::kJumperStatusCmd_ = "70";
const QString K8090::kFirmwareVersionCmd_ = "71";

/*!
    \class K8090
    \brief The class that provides the interface for Velleman %K8090 relay card
    controlling through serial port.

    \remark reentrant, thread-safe
*/

// initialization of static member variables
/*!
    \brief Array of 2 byte (1 leading byte and one command byte) commands used
    to control the relay.

    It is filled by fillCommandsArrays() static method, the first command is
    the command with the most important priority, the last is the least
    important. They should be accessed using the K8090Traits::Command enum
    values.

    Example, which shows how to build the whole command read status:

    \code
    int n = 7;  // Number of command bytes.
    unsigned char cmd[n]; // array of command bytes

    // copying the first two bytes of command to command byte array
    for (int ii = 0; ii < 2; ++ii)
        cmd[ii] = b_commands_[static_cast<int>(Command::RELAY_STATUS)][ii];
    cmd[2] = static_cast<unsigned char>(RelayID::ONE);  // 3rd byte specifies affected relays
    // commands, there is no one.
    cmd[5] = K8090::checkSum(cmd, 5); // sixth byte contains check sum.
    cmd[6] = b_etx_byte_;
    \endcode
*/
unsigned char K8090::b_commands_[static_cast<int>(Command::NONE)][2];

/*!
    \brief End delimiting byte.
*/
unsigned char K8090::b_etx_byte_;

/*!
    \brief Array of QString representation of command bytes used to control
    the relay card.

    To assemble the full command, it is necessary to prepend the kStxByte
    and append byte, which specifies the relays to be used, folowed by bytes
    containing data and ended with checksum (See CheckSum(const unsigned char,
    int)) and kEtxByte. It is filled by fillCommandsArrays() static method.
    They should be accessed using the K8090Traits::Command enum values.
    Example, which shows how to build the whole command _query relay status_:
    \code
    QString strCmd(kStxByte);
    int n = 0; // number of data bytes
    strCmd.append(strCommands[static_cast<int>(Command::RELAY_STATUS)])
          .append(" 00 00 00 ")
          .append(checkSum(strCmd))
          .append(QString(" %1".arg(kEtxByte_)));
    \endcode
*/
QString K8090::str_commands_[static_cast<int>(Command::NONE)];


/*!
  \brief Creates a new K8090 instance and sets the default values.
*/
K8090::K8090(QObject *parent) :
    QObject(parent)
{
    last_command_ = Command::NONE;
    fillCommandsArrays();

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
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = b_commands_[static_cast<int>(Command::RELAY_ON)][ii];
    cmd[2] = (unsigned char) RelayID::ONE;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = b_etx_byte_;
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
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = b_commands_[static_cast<int>(Command::RELAY_OFF)][ii];
    cmd[2] = (unsigned char) RelayID::ONE;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = b_etx_byte_;
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
        for (int ii = 0; ii < 2; ++ii)
            if (bMsg[ii] != b_commands_[static_cast<int>(cmd)][ii])
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
   \brief Fills arrays containing commands according to their importance.
 */
void K8090::fillCommandsArrays()
{
    str_commands_[static_cast<int>(Command::RELAY_ON)] = kSwitchRelayOnCmd_;
    str_commands_[static_cast<int>(Command::RELAY_OFF)] = kSwitchRelayOffCmd_;
    str_commands_[static_cast<int>(Command::TOGGLE_RELAY)] = kToggleRelayCmd_;
    str_commands_[static_cast<int>(Command::RELAY_STATUS)] = kQueryRelayStatusCmd_;
    str_commands_[static_cast<int>(Command::SET_BUTTON_MODE)] = kSetButtonModeCmd_;
    str_commands_[static_cast<int>(Command::BUTTON_MODE)] = kQueryButtonModeCmd_;
    str_commands_[static_cast<int>(Command::START_TIMER)] = kStartRelayTimerCmd_;
    str_commands_[static_cast<int>(Command::SET_TIMER)] = kSetRelayTimerDelayCmd_;
    str_commands_[static_cast<int>(Command::TIMER)] = kQueryTimerDelayCmd_;
    str_commands_[static_cast<int>(Command::BUTTON_STATUS)] = kButtonStatusCmd_;
    str_commands_[static_cast<int>(Command::RELAY_STATUS)] = kRelayStatusCmd_;
    str_commands_[static_cast<int>(Command::RESET_FACTORY_DEFAULTS)] = kResetFactoryDefaultsCmd_;
    str_commands_[static_cast<int>(Command::JUMPER_STATUS)] = kJumperStatusCmd_;
    str_commands_[static_cast<int>(Command::FIRMWARE_VERSION)] = kFirmwareVersionCmd_;

    bool ok;
    for (int ii = 0; ii < static_cast<int>(Command::NONE); ++ii) {
        b_commands_[ii][0] = kStxByte_.toUInt(&ok, 16);
        b_commands_[ii][1] = static_cast<unsigned char>(str_commands_[ii].toUInt(&ok, 16));
    }

    b_etx_byte_ = kEtxByte_.toUInt(&ok, 16);
}


/*!
   \brief K8090::~K8090
 */
K8090::~K8090()
{
    serial_port_->close();
    delete serial_port_;
}
