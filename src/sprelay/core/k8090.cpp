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
    timer_ = new QTimer(this);
    connect(timer_, &QTimer::timeout, this, &K8090::onFailedAttemptForConnection);
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
/*!
 * \brief K8090::onFailedAttemptForConnection
 */
void K8090::onFailedAttemptForConnection()
{
    number_of_failed_attemps_for_connection_++;
    if (number_of_failed_attemps_for_connection_ > 3)
    {
        disconnect();
    }else{
        timer_->stop();
        timer_->start(300);
    }
}
/*!
 * \brief K8090::connectK8090
 * \brief
 *  Function controll if is connected some device and also if is used right device. After that are set port characteristics (port name,
 * baud rate, data bits and others). Then are send Relay State, Button State and Query Timer Delay commands to test connection and also
 * to initialize private variable related to card status.
 */
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
    connection();
}
/*!
 * \brief K8090::connection
 * \brief Function will send queryRelayStatus, queryButtonModes, and queryTotalTimerDelay commands for each relay.
 */
void K8090::connection()
{
    queryRelayStatus();
    queryButtonModes();
    queryTotalTimerDelay(Relays::One);
    queryTotalTimerDelay(Relays::Two);
    queryTotalTimerDelay(Relays::Three);
    queryTotalTimerDelay(Relays::Four);
    queryTotalTimerDelay(Relays::Five);
    queryTotalTimerDelay(Relays::Six);
    queryTotalTimerDelay(Relays::Seven);
    queryTotalTimerDelay(Relays::Eight);
}
/*!
 * \brief K8090::disconnect
 */
void K8090::disconnect()
{
    qDebug() << "I will disconnect card";
}
/*!
 * \brief K8090::switchRelayOn
 * \param relays
 * \brief
 * Function will switch optional relays on. If there is some command pending, function will save byte array to
 * the priority queue \b (stored_commands_priority_queue).
 * \note On command card response with event command 50h
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.4
 * \see stored_commands_priority_queue
 */
void K8090::switchRelayOn(K8090Traits::Relays relays)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SwitchRelayOn)][ii];
    cmd[2] = (unsigned char)relays;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::SwitchRelayOn;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure switch_relay_on_cmd;
        switch_relay_on_cmd.cmd = cmd;
        switch_relay_on_cmd.priority = 1;
        stored_command_priority_queue.push(switch_relay_on_cmd);
        qDebug() << "new item in list";
    }
}
/*!
 * \brief K8090::switchRelayOff
 * \param relays
 * \brief
 * Function will switch optional relays off. If there is some command pending, function will save byte array to
 * the priority queue \b (stored_commands_priority_queue).
 * \note On command card response with event command 50h
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.4
 * \see stored_commands_priority_queue
 */
void K8090::switchRelayOff(K8090Traits::Relays relays)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SwitchRelayOff)][ii];
    cmd[2] = (unsigned char)relays;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::SwitchRelayOff;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure switch_relay_off_cmd;
        switch_relay_off_cmd.cmd = cmd;
        switch_relay_off_cmd.priority = 1;
        stored_command_priority_queue.push(switch_relay_off_cmd);
        qDebug() << "New item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::toggleRelay
 * \param relays
 * \brief
 * Function will toggle optional relays. If there is some command pending, function will save byte array to
 * the priority queue \b (stored_commands_priority_queue).
 * \note On command card response with event command 50h
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.5
 * \see stored_commands_priority_queue
 */
void K8090::toggleRelay(K8090Traits::Relays relays)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ToggleRelay)][ii];
    cmd[2] = (unsigned char)relays;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::ToggleRelay;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure toggle_relay_cmd;
        toggle_relay_cmd.cmd = cmd;
        toggle_relay_cmd.priority = 1;
        stored_command_priority_queue.push(toggle_relay_cmd);
        qDebug() << "new item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::setButtonMode
 * \param momentary
 * \param toggle
 * \param timed
 * \brief
 * Function will set button mode for optional relays. On Relays can be set Momentary, Toggle and Timed mode.
 *     In \b Momentary mode are relays turned on while is button pressed.
 *
 *     In \b Toggle mode are afred using correct button relays Toggled.
 *
 *     In \b Timed mode is after using specific button, on specfic relay timer started.
 * If there is some command pending, byte array is stored in priority queue \b (stored_commands_priority_queue).
 * \note Card don't response with any packet.
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.6
 * \see stored_commands_priority_queue
 */
void K8090::setButtonMode(K8090Traits::Relays momentary, K8090Traits::Relays toggle, K8090Traits::Relays timed)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetButtonMode)][ii];
    cmd[2] = (unsigned char)momentary;
    cmd[3] = (unsigned char)toggle;
    cmd[4] = (unsigned char)timed;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::SetButtonMode;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure set_Button_Mode_cmd;
        set_Button_Mode_cmd.cmd = cmd;
        set_Button_Mode_cmd.priority = 1;
        stored_command_priority_queue.push(set_Button_Mode_cmd);
        qDebug() << "New item in the list.";
    }
}
/*!
 * \brief K8090::startRelayTimer
 * \param relays
 * \param delay
 * \brief
 * Function will start on specific relay/s timer for optional time. If there is some command pending, byte array
 * will be stored in priority queue \b (stored_commands_priority_queue). For decomposition of  usingned integer to
 * two bytes is used \b lowByt and \b highByt.
 * \note Card response with event command 50h
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.7
 * \see stored_commands_priority_queue
 * \see lowByt
 * \see highByt
 */
void K8090::startRelayTimer(K8090Traits::Relays relays, unsigned int delay)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::StartRelayTimer)][ii];
    cmd[2] = (unsigned char)relays;
    cmd[3] = highByt(delay);
    cmd[4] = lowByt(delay);
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::StartRelayTimer;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure start_relay_timer_cmd;
        start_relay_timer_cmd.cmd = cmd;
        start_relay_timer_cmd.priority = 1;
        stored_command_priority_queue.push(start_relay_timer_cmd);
        qDebug() << "new item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::setRelayTimerDelay
 * \param relays
 * \param delay
 * \brief
 * Function will set on optional relay timer specific time.
 * For conversion unsigned int to 2 bytes are used methods \b highByt and \b lowByt
 * \note Card don't response with any packet.
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.7
 * \see stored_commands_priority_queue
 * \see lowByt
 * \see highByt
 */
void K8090::setRelayTimerDelay(K8090Traits::Relays relays, unsigned int delay)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::SetRelayTimerDelay)][ii];
    cmd[2] = (unsigned char)relays;
    cmd[3] = highByt(delay);
    cmd[4] = lowByt(delay);
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::SetRelayTimerDelay;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        onSendToSerial(cmd, n);
        sendNextCommand();
    }else{
        stored_command_structure set_Relay_Timer_Delay_cmd;
        set_Relay_Timer_Delay_cmd.cmd = cmd;
        set_Relay_Timer_Delay_cmd.priority = 1;
        stored_command_priority_queue.push(set_Relay_Timer_Delay_cmd);
        qDebug() << "New item in the list.";
    }
}
/*!
 * \brief K8090::queryRelayStatus
 * \brief
 * Function will send request to card for information about status of relays. If there is somecommand pending,
 * it will be stored in priority queue. Card should send packet with information about turn on/off relays.
 * \note Card response with packet 50h.
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.8
 * \see stored_commands_priority_queue
 */
void K8090::queryRelayStatus()
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::QueryRelayStatus)][ii];
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::QueryRelayStatus;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure query_Relay_Status_cmd;
        query_Relay_Status_cmd.cmd = cmd;
        query_Relay_Status_cmd.priority = 1;
        stored_command_priority_queue.push(query_Relay_Status_cmd);
        qDebug() << "new item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::queryRemainingTimerDelay
 * \param relays
 * \brief
 * Function will send request to card about remaining timer delay on specific relay
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.9
 * \see stored_commands_priority_queue
 */
void K8090::queryRemainingTimerDelay(K8090Traits::Relays relays)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    wanted_total_timer_delay_ = false;
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::QueryTimerDelay)][ii];
    cmd[2] = (unsigned char)relays;
    cmd[3] = (unsigned char)K8090Traits::Relays::Two;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::QueryTimerDelay;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure query_Remaining_timer_delay_cmd;
        query_Remaining_timer_delay_cmd.cmd = cmd;
        query_Remaining_timer_delay_cmd.priority = 1;
        stored_command_priority_queue.push(query_Remaining_timer_delay_cmd);
        qDebug() << "new item in list";
    }
    timer_->start(500);
}

/*!
 * \brief K8090::queryRemainingTimerDelay
 * \param relays
 * \brief
 * Function will send request to card about total timer delay on specific relay
 * \note card response with packet 44h
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.9
 * \see stored_commands_priority_queue
 */
void K8090::queryTotalTimerDelay(K8090Traits::Relays relays)
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    wanted_total_timer_delay_ = true;
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::QueryTimerDelay)][ii];
    cmd[2] = (unsigned char)relays;
    cmd[3] = (unsigned char)K8090Traits::Relays::One;
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::QueryTimerDelay;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure query_Total_timer_delay_cmd;
        query_Total_timer_delay_cmd.cmd = cmd;
        query_Total_timer_delay_cmd.priority = 1;
        stored_command_priority_queue.push(query_Total_timer_delay_cmd);
         qDebug() << "new item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::queryButtonModes
 * \brief
 * Function will send request to card about button mode, which is set on specific relay.
 * \note card response with packet 44h
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.10
 * \see stored_commands_priority_queue
 */
void K8090::queryButtonModes()
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::QueryButtonMode)][ii];
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::QueryButtonMode;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure query_Button_States_cmd;
        query_Button_States_cmd.cmd = cmd;
        query_Button_States_cmd.priority = 1;
        stored_command_priority_queue.push(query_Button_States_cmd);
        qDebug() << "new item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::resetFactoryDefauts
 * \brief
 * Reset the board to factory defaults.All buttons are set to toggle mode and all timer delays are set to 5 seconds.
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.12
 * \see stored_commands_priority_queue
 */
void K8090::resetFactoryDefauts()
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::ResetFactoryDefaults)][ii];
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::ResetFactoryDefaults;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure reset_Factory_Defaults_cmd;
        reset_Factory_Defaults_cmd.cmd = cmd;
        reset_Factory_Defaults_cmd.priority = 1;
        stored_command_priority_queue.push(reset_Factory_Defaults_cmd);
        qDebug() << "New item in the list.";
    }
}
/*!
 * \brief K8090::queryJumperStatus
 * \brief
 * Function will send request about Jumper.If the jumper is set, the buttons no longer interact with the
relays but button events are still sent to the computer.
 * \note in future develop can be used to solve problem with occasional problems with connection (For example
 * when comunication fail, we can set jumper status, press some button and communication should works)
 * \see K8090/VM8090 Protocol Manual. Technical Guide. p.13
 * \see stored_commands_priority_queue
 */
void K8090::queryJumperStatus()
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::JumperStatus)][ii];
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::JumperStatus;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure jumper_Status_cmd;
        jumper_Status_cmd.cmd = cmd;
        jumper_Status_cmd.priority = 1;
        stored_command_priority_queue.push(jumper_Status_cmd);
        qDebug() << "new item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::queryFirmwareVersion
 * \brief
 * Functio will send request related to Firmware version of the card. The version number consists of the year and week
combination of the date the firmware was compiled.
 * \see K8090/VM8090 Protocol Manual. Technical Guide.
 * \see stored_commands_priority_queue
 */
void K8090::queryFirmwareVersion()
{
    int n = 7;  // Number of command bytes.
    unsigned char * cmd = new unsigned char[n];
    int ii;
    for (ii = 0; ii < 2; ++ii)
        cmd[ii] = bCommands[static_cast<int>(Command::FirmwareVersion)][ii];
    cmd[5] = checkSum(cmd, 5);
    cmd[6] = bEtxByte;
    lastCommand = Command::FirmwareVersion;
    qDebug() << byteToHex(cmd, n);
    if (command_finished_)
    {
        command_finished_ = false;
        onSendToSerial(cmd, n);
    }else{
        stored_command_structure firmware_Version_cmd;
        firmware_Version_cmd.cmd = cmd;
        firmware_Version_cmd.priority = 1;
        stored_command_priority_queue.push(firmware_Version_cmd);
        qDebug() << "new item in list";
    }
    timer_->start(500);
}
/*!
 * \brief K8090::refreshRelayStates
 * \param previous
 * \param current
 * \param timed
 * \brief
 *  According to packet from the card, function will determine, which relays are turned on.
 * \note In fuction is used formula \b (switched on = (previous^current)^current))
 * \see K8090/VM8090 Protocol Manual. Technical Guide.
 * \see onSendToSerial
 */

void K8090::refreshRelayStates(unsigned char previous, unsigned char current, unsigned char timed)
{
  relay_states_ = (relay_states_|((previous^current)&current))^((previous^current)&previous);
}

/*!
 * \brief K8090::refreshButtonMode
 * \param momentary
 * \param toggle
 * \param timed
 * \brief
 * According to packet from the card, function will in QDebug show, on which button are momentary, timed or toggle mode set.
 * These numbers in binary represent , same as with relays, specific buttons.
 * \see K8090/VM8090 Protocol Manual. Technical Guide.
 */

void K8090::refreshButtonMode(const unsigned char momentary, const unsigned char toggle, const unsigned char timed)
{
    momentary_button_mode_ = (momentary_button_mode_ | momentary) & momentary;
    toggle_button_mode_ = (toggle_button_mode_ | toggle) & toggle;
    timed_button_mode_ = (timed_button_mode_ | timed) & timed;
}

/*!
 * \brief K8090::onButtonStatus
 * \param isPressed
 * \param hasBeenPressed
 * \param hasBeenReleased
 * \brief
 * According to packet from the card, will function show in QDebug which button has been pressed,
 * is pressed or have been released.
 * \see K8090/VM8090 Protocol Manual. Technical Guide.
 */
void K8090::onButtonStatus(unsigned char isPressed, unsigned char hasBeenPressed, unsigned char hasBeenReleased)
{
    unsigned char testing_number = 0;
    int i = 0;
    for (i = 0; i < 8; i++)
    {
        testing_number = 1 << i;
        if (!((isPressed & testing_number) == 0))
        {
            qDebug() << "Button " << i+1 << " is pressed.";
        }
        if (!((hasBeenPressed & testing_number) == 0))
        {
            qDebug() << "Button " << i+1 << " has been pressed.";
        }
        if (!((hasBeenReleased & testing_number) == 0))
        {
            qDebug() << "Button " << i+1 << " has been released.";
        }
    }
}
/*!
 * \brief K8090::onTimerDelay
 * \param Relays
 * \param highbyt
 * \param lowbyt
 * \brief
 * Function will in QDebug show total or remaining delay time for specific relay.
 * \see K8090/VM8090 Protocol Manual. Technical Guide.
 */
void K8090::onTimerDelay(unsigned char Relays, unsigned char highbyt, unsigned char lowbyt)
{
    unsigned char testing_number = 0;
    int i = 0;
    qDebug() << "Relays ";
    for (i = 0; i < 8; i++)
    {
        testing_number = 1 << i;
        if (!((Relays & testing_number) == 0))
        {
            qDebug() << i+1 << ", ";
        }
    }
        qDebug() << "has ";
        if (wanted_total_timer_delay_)
        {
            qDebug() << "total timer delay is ";
        }else{
            qDebug() << "remaining timer delay is ";
        }
        unsigned int time = 0;
        unsigned int highbytint = 0;
        unsigned int lowbytint = 0;
        highbytint =  (static_cast<unsigned int>(highbyt)) << 7;
        lowbytint =  (static_cast<unsigned int>(lowbyt));
        time = highbytint|(time|lowbytint);
        qDebug() << time;
}

/*!
 * \brief K8090::onSendToSerial()
 * \param const unsigned char *buffer
 * \param int n
 * \brief
 *  Function will send array of byte to serial port
 */
void K8090::onSendToSerial(const unsigned char *buffer, int n)
{
    qDebug() <<"Sended to serial port " << byteToHex(buffer, n);
    if (!serialPort_->isOpen())
        serialPort_->open(QIODevice::ReadWrite);
    serialPort_->write(reinterpret_cast<char*>(const_cast<unsigned char*>(buffer)), n);
    delete[] buffer;
}
/*!
 * \brief K8090::onReadyData
 * \brief Function will data from relay card and according to them,
 * \b (if they will pass bz checksum control) will emit specific signals.
*/
void K8090::onReadyData()
{
    qDebug() << "R8090::onReadyData";  // converting the data to unsigned char
    QByteArray data = serialPort_->readAll();
    int n = data.size();
    timer_->stop();
    unsigned char *buffer = reinterpret_cast<unsigned char*>(data.data());
    if (checkSum(buffer, 5) == buffer[5])
    {
        qDebug() << "Packet is allright.";
        qDebug() << byteToHex(buffer, n);
        switch (buffer[1])
        {
        case 0x51: qDebug() << static_cast<int>(relay_states_);
            emit relayStatus(buffer[2], buffer[3], buffer[4]);
            qDebug() << static_cast<int>(relay_states_);
            if (!stored_command_priority_queue.empty())
            {
                stored_command_structure cmd2 = stored_command_priority_queue.top();
                if (cmd2.cmd[1] == 0x42)
                {
                    qDebug() << "yes";
                  onSendToSerial(cmd2.cmd, 7);
                  stored_command_priority_queue.pop();
                  sendNextCommand();
                }else{
                    if (cmd2.cmd[1] == 0x21)
                    {
                        qDebug() << "yes";
                      onSendToSerial(cmd2.cmd, 7);
                      stored_command_priority_queue.pop();
                      sendNextCommand();
                    }else{
                        if (cmd2.cmd[1] == 0x66)
                        {
                            qDebug() << "yes";
                          onSendToSerial(cmd2.cmd, 7);
                          stored_command_priority_queue.pop();
                          sendNextCommand();
                        }else{
                    onSendToSerial(cmd2.cmd, 7);
                    stored_command_priority_queue.pop();
                        }
                    }
                }
                 qDebug() << "removed item." << stored_command_priority_queue.size();
            }else{
                command_finished_ = true;
            }
            break;  // hexadecimálneho zápisu 51
        case 0x22: qDebug() << static_cast<int>(momentary_button_mode_);
            qDebug() << static_cast<int>(toggle_button_mode_);
            qDebug() << static_cast<int>(timed_button_mode_);
            emit refreshButtonMode(buffer[2], buffer[3], buffer[4]);
            qDebug() << static_cast<int>(momentary_button_mode_);
            qDebug() << static_cast<int>(toggle_button_mode_);
            qDebug() << static_cast<int>(timed_button_mode_);
            if (!stored_command_priority_queue.empty())
            {stored_command_structure cmd2 = stored_command_priority_queue.top();
                if (cmd2.cmd[1] == 0x42)
                {
                    qDebug() << "yes";
                  onSendToSerial(cmd2.cmd, 7);
                  stored_command_priority_queue.pop();
                  sendNextCommand();
                }else{
                    if (cmd2.cmd[1] == 0x21)
                    {
                        qDebug() << "yes";
                      onSendToSerial(cmd2.cmd, 7);
                      stored_command_priority_queue.pop();
                      sendNextCommand();
                    }else{
                        if (cmd2.cmd[1] == 0x66)
                        {
                            qDebug() << "yes";
                          onSendToSerial(cmd2.cmd, 7);
                          stored_command_priority_queue.pop();
                          sendNextCommand();
                        }else{
                    onSendToSerial(cmd2.cmd, 7);
                    stored_command_priority_queue.pop();
                        }
                    }
                }
                 qDebug() << "removed item." << stored_command_priority_queue.size();
            }else{
                command_finished_ = true;
            }
            break;
        case 0x50: emit buttonStatus(buffer[2], buffer[3], buffer[4]);
            break;
        case 0x44: emit timerDelay(buffer[2], buffer[3], buffer[4]);
            if (!stored_command_priority_queue.empty())
            {stored_command_structure cmd2 = stored_command_priority_queue.top();
                if (cmd2.cmd[1] == 0x42)
                {
                    qDebug() << "yes";
                  onSendToSerial(cmd2.cmd, 7);
                  stored_command_priority_queue.pop();
                  sendNextCommand();
                }else{
                    if (cmd2.cmd[1] == 0x21)
                    {
                        qDebug() << "yes";
                      onSendToSerial(cmd2.cmd, 7);
                      stored_command_priority_queue.pop();
                      sendNextCommand();
                    }else{
                        if (cmd2.cmd[1] == 0x66)
                        {
                            qDebug() << "yes";
                          onSendToSerial(cmd2.cmd, 7);
                          stored_command_priority_queue.pop();
                          sendNextCommand();
                        }else{
                    onSendToSerial(cmd2.cmd, 7);
                    stored_command_priority_queue.pop();
                        }
                    }
                }
                 qDebug() << "removed item." << stored_command_priority_queue.size();
            }else{
                command_finished_ = true;
            }
            break;
        default: qDebug() << "Your choice is not recognized.";
        }
    }
    lastCommand = Command::None;
}

unsigned char K8090::lowByt(unsigned int number)
{
    unsigned char bytarr[2];
    bytarr[0] = (number)&(0xFF);
    bytarr[1] = (number>>8)&(0xFF);
    return bytarr[0];
}
/*!
  *\brief K8090::lowByt(number)
  *\brief Save first 8 bits of 16 bit unsigned integer.
  */
unsigned char K8090::highByt(unsigned int number)
{
    unsigned char bytarr[2];
    bytarr[0] = number&0xFF;
    bytarr[1] = (number>>8)&0xFF;
    return bytarr[1];
}
/*!
  *\brief K8090::highByt(unsigned int number)
  *\brief Save second 8 bits of 16 bit unsigned integer.
  */

/*!
 * \brief K8090::sendNextCommand
 * \brief
 * Despite the fact, that some commands don't have response, program should emit signal disconnect.
 * However this function ensure, that if connection is alright, and there are som other commands pending,
 * next command will be sent.
 */
void K8090::sendNextCommand()
{
    if (!stored_command_priority_queue.empty())
    {
        qDebug() << "Send next cmd";
        stored_command_structure next_command = stored_command_priority_queue.top();
        stored_command_priority_queue.pop();
        onSendToSerial(next_command.cmd, 7);
        next_command = stored_command_priority_queue.top();
        if (next_command.cmd[1] == 0x42)
        {
        sendNextCommand();
        }else{
            if (next_command.cmd[1] == 0x21)
            {
            sendNextCommand();
            }else{
                if (next_command.cmd[1] == 0x66)
                {
                sendNextCommand();
                }else{
                 if (!stored_command_priority_queue.empty())
                 {
                 onSendToSerial(next_command.cmd, 7);
                 stored_command_priority_queue.pop();
                 }
                }
            }
       }
    }else{
      qDebug() <<  "I'm empty!!!";
    }
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
 * \brief K8090::byteToHex
 * \param buffer
 * \param n
 * \brief
 * This function will translate byte array (array of unsigned char) to haxedecimal numeric system.
 * \return QString
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
