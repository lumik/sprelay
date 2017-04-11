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

#ifndef CORE_K8090_H_
#define CORE_K8090_H_

#include <QObject>

// forward declarations
class QSerialPort;

namespace K8090Traits
{
enum class Command {
    SwitchRelayOn,
    SwitchRelayOff,
    ToggleRelay,
    QueryRelayStatus,
    SetButtonMode,
    QueryButtonMode,
    StartRelayTimer,
    SetRelayTimerDelay,
    QueryTimerDelay,
    ButtonStatus,
    RelayStatus,
    ResetFactoryDefaults,
    JumperStatus,
    FirmwareVersion,
    None
};

inline Command& operator++(Command &cmd) { cmd = static_cast<Command>(static_cast<int>(cmd) + 1); return cmd; }
inline Command operator++(Command &cmd, int) { Command tmp(cmd); cmd++; return tmp; }

enum struct RelayID : unsigned char
{
    None  = 0,
    One   = 1 << 0,
    Two   = 1 << 1,
    Three = 1 << 2,
    Four  = 1 << 3,
    Five  = 1 << 4,
    Six   = 1 << 5,
    Seven = 1 << 6,
    Eight = 1 << 7
};

inline constexpr RelayID operator|(RelayID a, RelayID b) {
    return static_cast<RelayID>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }
inline constexpr RelayID operator&(RelayID a, RelayID b) {
    return static_cast<RelayID>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }
inline RelayID operator~(RelayID a) { return static_cast<RelayID>(~static_cast<unsigned char>(a)); }
inline RelayID& operator|=(RelayID& a, RelayID b) { return a = (a | b); }
inline RelayID& operator&=(RelayID& a, RelayID b) { return a = (a & b); }

struct ComPortParams
{
    QString portName;
    QString description;
    QString manufacturer;
    quint16 productIdentifier;
    quint16 vendorIdentifier;
};
}  // namespace K8090Traits

class K8090 : public QObject
{
    Q_OBJECT
public:  // NOLINT(whitespace/indent)
    explicit K8090(QObject *parent = 0);
    virtual ~K8090();

    static QList<K8090Traits::ComPortParams> availablePorts();


signals:  // NOLINT(whitespace/indent)

public slots:  // NOLINT(whitespace/indent)
    void connectK8090();

protected slots:  // NOLINT(whitespace/indent)
    void onSendToSerial(const unsigned char *buffer, int n);

private slots:  // NOLINT(whitespace/indent)
    void onReadyData();

private:  // NOLINT(whitespace/indent)
    void sendCommand();

    void sendSwitchRelayOnCommand();
    void sendSwitchRelayOffCommand();

    static const quint16 productID;
    static const quint16 vendorID;

    QString comPortName_;
    QSerialPort *serialPort_;

    static void hexToByte(unsigned char **pbuffer, int *n, const QString &msg);
    static QString byteToHex(const unsigned char *buffer, int n);
    static QString checkSum(const QString &msg);
    static unsigned char checkSum(const unsigned char *bMsg, int n);
    static bool validateResponse(const QString &msg, K8090Traits::Command cmd);
    static bool validateResponse(const unsigned char *bMsg, int n, K8090Traits::Command cmd);

    static void fillCommandsArrays();

    bool commandBuffer[static_cast<int>(K8090Traits::Command::None)];

    K8090Traits::Command lastCommand;

    static unsigned char bCommands[static_cast<int>(K8090Traits::Command::None)][2];
    static unsigned char bEtxByte;
    static QString strCommands[static_cast<int>(K8090Traits::Command::None)];

    static const QString stxByte;
    static const QString etxByte;
    static const QString switchRelayOnCmd;
    static const QString switchRelayOffCmd;
    static const QString toggleRelayCmd;
    static const QString queryRelayStatusCmd;
    static const QString setButtonModeCmd;
    static const QString queryButtonModeCmd;
    static const QString startRelayTimerCmd;
    static const QString setRelayTimerDelayCmd;
    static const QString queryTimerDelayCmd;
    static const QString buttonStatusCmd;
    static const QString relayStatusCmd;
    static const QString resetFactoryDefaultsCmd;
    static const QString jumperStatusCmd;
    static const QString firmwareVersionCmd;
};

#endif  // CORE_K8090_H_
