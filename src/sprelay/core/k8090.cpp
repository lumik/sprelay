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

#include "k8090.h"

#include <QSerialPort>
#include <QSerialPortInfo>

#include <QStringBuilder>

#include <QDebug>

/*!
    \file k8090.h
*/

/*!
    \namespace K8090Traits
    \brief Contains K8090 traits
*/
using namespace K8090Traits;  // NOLINT(build/namespaces)

// static constants
const quint16 K8090::productID = 32912;
const quint16 K8090::vendorID = 4303;

const QString K8090::stxByte = "04";
const QString K8090::etxByte = "0F";
const QString K8090::switchRelayOnCmd = "11";
const QString K8090::switchRelayOffCmd = "12";
const QString K8090::toggleRelayCmd = "14";
const QString K8090::queryRelayStatusCmd = "18";
const QString K8090::setButtonModeCmd = "21";
const QString K8090::queryButtonModeCmd = "22";
const QString K8090::startRelayTimerCmd = "41";
const QString K8090::setRelayTimerDelayCmd = "42";
const QString K8090::queryTimerDelayCmd = "44";
const QString K8090::buttonStatusCmd = "50";
const QString K8090::relayStatusCmd = "51";
const QString K8090::resetFactoryDefaultsCmd = "66";
const QString K8090::jumperStatusCmd = "70";
const QString K8090::firmwareVersionCmd = "71";

/*!
    \class K8090
    \brief The class that provides the interface for %Velleman K8090 relay card
    controlling through serial port.

    \remark reentrant, thread-safe
*/

// initialization of static member variables
/*!
    \brief Array of 4 byte (3 leading bytes and one command byte) commands used
    to control the relay.

    It is filled by fillCommandsArrays() static method, the first command is
    the command with the most important priority, the last is the least
    important. They should be accessed using the K8090Traits::Command enum
    values.

    Example, which shows how to build the whole command read status:

    \code
    int n = 6; // number of command bytes
    unsigned char cmd[n]; // array of command bytes

    // copying the first two bytes of command to command byte array
    for (int ii = 0; ii < 4; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ReadStatus)][ii];
    cmd[4] = 0; // 5th byte specifies the number of data bytes. For read
    // commands, there is no one.
    cmd[5] = K8090::checkSum(cmd, 5); // the last byte contains check sum.
    \endcode
*/
unsigned char K8090::bCommands[static_cast<int>(Command::None)][2];

unsigned char K8090::bEtxByte;

/*!
    \brief Array of QString representation of command bytes used to control the
    bath.

    To assemble the full command, it is necessary to prepend the commandBase
    and append byte, which contains number of data bytes, folowed bytes
    containing data and ended with checksum (See CheckSum(const unsigned char,
    int)). It is filled by fillCommandsArrays() static method, the first
    command is the command with the most important priority, the last is the
    least important. They should be accessed using the K8090Traits::Command
    enum values. Example, which shows how to build the whole command Read
    status:
    \code
    QString strCmd(commandBase);
    int n = 0; // number of data bytes
    strCmd.append(strCommands[static_cast<int>(Command::ReadStatus)])
          .append(QString(" %1 ").arg(n, 2, 16, QChar('0')).toUpper());
    strCmd.append(checkSum(strCmd));
    \endcode
*/
QString K8090::strCommands[static_cast<int>(Command::None)];

/*!
  \brief Creates a new K8090 instance and sets the default values.
*/
K8090::K8090(QObject *parent) :
    QObject(parent)
{
    lastCommand = Command::None;
    fillCommandsArrays();

    serialPort_ = new QSerialPort(this);
    connect(serialPort_, &QSerialPort::readyRead, this, &K8090::onReadyData);
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
        if (info.productIdentifier() == productID &&
                info.vendorIdentifier() == vendorID) {
            cardFound = true;
            comPortName_ = info.portName();
            qDebug() << "Port name: " % comPortName_;
        }
    }

    if (!cardFound) {
        qDebug() << "Card not found!!!";
        return;
    }

    serialPort_->setPortName(comPortName_);
    serialPort_->setBaudRate(QSerialPort::Baud19200);
    serialPort_->setDataBits(QSerialPort::Data8);
    serialPort_->setParity(QSerialPort::NoParity);
    serialPort_->setStopBits(QSerialPort::OneStop);
    serialPort_->setFlowControl(QSerialPort::NoFlowControl);

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
    if (!serialPort_->isOpen())
        serialPort_->open(QIODevice::ReadWrite);
    serialPort_->write(reinterpret_cast<char*>(const_cast<unsigned char*>(buffer)), n);
    delete[] buffer;
}

void K8090::onReadyData()
{
    qDebug() << "R8090::onReadyData";

    // converting the data to unsigned char
    QByteArray data = serialPort_->readAll();
    int n = data.size();
    unsigned char *buffer = reinterpret_cast<unsigned char*>(data.data());
    qDebug() << byteToHex(buffer, n);

    lastCommand = Command::None;
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
        cmd[ii] = bCommands[static_cast<int>(Command::SwitchRelayOn)][ii];
    cmd[2] = (unsigned char) RelayID::One;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::SwitchRelayOn;
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
        cmd[ii] = bCommands[static_cast<int>(Command::SwitchRelayOff)][ii];
    cmd[2] = (unsigned char) RelayID::One;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::SwitchRelayOff;
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
            if (bMsg[ii] != bCommands[static_cast<int>(cmd)][ii])
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
    strCommands[static_cast<int>(Command::SwitchRelayOn)] = switchRelayOnCmd;
    strCommands[static_cast<int>(Command::SwitchRelayOff)] = switchRelayOffCmd;
    strCommands[static_cast<int>(Command::ToggleRelay)] = toggleRelayCmd;
    strCommands[static_cast<int>(Command::QueryRelayStatus)] = queryRelayStatusCmd;
    strCommands[static_cast<int>(Command::SetButtonMode)] = setButtonModeCmd;
    strCommands[static_cast<int>(Command::QueryButtonMode)] = queryButtonModeCmd;
    strCommands[static_cast<int>(Command::StartRelayTimer)] = startRelayTimerCmd;
    strCommands[static_cast<int>(Command::SetRelayTimerDelay)] = setRelayTimerDelayCmd;
    strCommands[static_cast<int>(Command::QueryTimerDelay)] = queryTimerDelayCmd;
    strCommands[static_cast<int>(Command::ButtonStatus)] = buttonStatusCmd;
    strCommands[static_cast<int>(Command::RelayStatus)] = relayStatusCmd;
    strCommands[static_cast<int>(Command::ResetFactoryDefaults)] = resetFactoryDefaultsCmd;
    strCommands[static_cast<int>(Command::JumperStatus)] = jumperStatusCmd;
    strCommands[static_cast<int>(Command::FirmwareVersion)] = firmwareVersionCmd;

    bool ok;
    for (int ii = 0; ii < static_cast<int>(Command::None); ++ii) {
        bCommands[ii][0] = stxByte.toUInt(&ok, 16);
        bCommands[ii][1] = static_cast<unsigned char>(strCommands[ii].toUInt(&ok, 16));
    }

    bEtxByte = etxByte.toUInt(&ok, 16);
}

/*!
   \brief K8090::~K8090
 */
K8090::~K8090()
{
    serialPort_->close();
    delete serialPort_;
}
