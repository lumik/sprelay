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

#ifndef SPRELAY_CORE_K8090_H_
#define SPRELAY_CORE_K8090_H_

#include <QObject>
#include<QTimer>
#include<queue>
#include<vector>

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
/*!
 * \brief operator ++
 * \param cmd
 * \return
 */
inline Command& operator++(Command &cmd) { cmd = static_cast<Command>(static_cast<int>(cmd) + 1); return cmd; }
/*!
 * \brief operator ++
 * \param cmd
 * \return
 */
inline Command operator++(Command &cmd, int) { Command tmp(cmd); cmd++; return tmp; }
/*!
 * \brief The Relays enum
 * \note enumerator is 0,1,2,4,8,16,32,64,128 (first relay has 1 on first position, 2 on second...)
*/
enum struct Relays : unsigned char
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
/*!
 * \brief operator |
 * \param a
 * \param b
 * \return
 */
inline constexpr Relays operator|(Relays a, Relays b) {
    return static_cast<Relays>(static_cast<unsigned char>(a) | static_cast<unsigned char>(b)); }
/*!
 * \brief operator &
 * \param a
 * \param b
 * \return
 */
inline constexpr Relays operator&(Relays a, Relays b) {
    return static_cast<Relays>(static_cast<unsigned char>(a) & static_cast<unsigned char>(b)); }
/*!
 * \brief operator ~
 * \param a
 * \return
 */
inline Relays operator~(Relays a) { return static_cast<Relays>(~static_cast<unsigned char>(a)); }
/*!
 * \brief operator |=
 * \param a
 * \param b
 * \return
 */
inline Relays& operator|=(Relays& a, Relays b) { return a = (a | b); }
/*!
 * \brief operator &=
 * \param a
 * \param b
 * \return
 */
inline Relays& operator&=(Relays& a, Relays b) { return a = (a & b); }
/*!
 * \brief The ComPortParams struct
 */
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
        void relayStatus(unsigned char previous, unsigned char current, unsigned char timed);
        void buttonStatus(unsigned char state, unsigned char pressed, unsigned char released);
        void timerDelay(unsigned char Relays,  unsigned char highbyt,  unsigned char lowbyt);
        void buttonMode(unsigned char momentary, unsigned char toggle, unsigned char timed);
        void jumperStatus(bool on);
        void firmwareVersion(int year, int week);
        void connected();
public slots:  // NOLINT(whitespace/indent)
    void onFailedAttemptForConnection();
    void connectK8090();
    void connection();
    void disconnect();
    void switchRelayOn(K8090Traits::Relays relays);
    void switchRelayOff(K8090Traits::Relays relays);
    void toggleRelay(K8090Traits::Relays relays);
    void setButtonMode(K8090Traits::Relays momentary, K8090Traits::Relays toggle, K8090Traits::Relays timed);
    void startRelayTimer(K8090Traits::Relays relays, unsigned int delay);
    void setRelayTimerDelay(K8090Traits::Relays relays, unsigned int delay);
    void queryRelayStatus();
    void queryRemainingTimerDelay(K8090Traits::Relays relays);
    void queryTotalTimerDelay(K8090Traits::Relays relays);
    void queryButtonModes();
    void resetFactoryDefauts();
    void queryJumperStatus();
    void queryFirmwareVersion();
    void refreshRelayStates(const unsigned char previous, const unsigned char current, const unsigned char timed);
    void refreshButtonMode(const unsigned char momentary, const unsigned char toggle, const unsigned char timed);
    void onButtonStatus(unsigned char isPressed, unsigned char hasBeenPressed, unsigned char hasBeenReleased);
    void onTimerDelay(unsigned char Relays,  unsigned char highbyt,  unsigned char lowbyt);


protected slots:  // NOLINT(whitespace/indent)
    void onSendToSerial(const unsigned char *buffer, int n);

private slots:  // NOLINT(whitespace/indent)
    void onReadyData();

private:  // NOLINT(whitespace/indent)    
    unsigned char lowByt(unsigned int number);
    unsigned char highByt(unsigned int number);
    void sendNextCommand();
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

    unsigned char relay_states_ = 0;
    unsigned char momentary_button_mode_ = 0;
    unsigned char toggle_button_mode_ = 0;
    unsigned char timed_button_mode_ = 0;
    bool command_finished_ = true;
    bool wanted_total_timer_delay_ = true;

    QTimer *timer_ = new QTimer;
    int number_of_failed_attemps_for_connection_ = 0;
    struct stored_command_structure
{
    unsigned char *cmd;
    int priority;
    bool operator<(const stored_command_structure& a)const
    {
        return priority < a.priority;
    }
    };

    std::priority_queue<stored_command_structure, std::vector<stored_command_structure>> stored_command_priority_queue;


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
#endif  // SPRELAY_CORE_K8090_H_
