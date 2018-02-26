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

#include <type_traits>

#include "enum_flags.h"

// forward declarations
class QSerialPort;

namespace sprelay {
namespace core {

namespace K8090Traits {
enum class Command : unsigned int
{
    RELAY_ON,
    RELAY_OFF,
    TOGGLE_RELAY,
    QUERY_RELAY,
    SET_BUTTON_MODE,
    BUTTON_MODE,
    START_TIMER,
    SET_TIMER,
    TIMER,
    BUTTON_STATUS,
    RELAY_STATUS,
    RESET_FACTORY_DEFAULTS,
    JUMPER_STATUS,
    FIRMWARE_VERSION,
    NONE
};


enum struct RelayID : unsigned char
{
    NONE  = 0,  /**< None relay */
    ONE   = 1 << 0,
    TWO   = 1 << 1,
    THREE = 1 << 2,
    FOUR  = 1 << 3,
    FIVE  = 1 << 4,
    SIX   = 1 << 5,
    SEVEN = 1 << 6,
    EIGHT = 1 << 7,
    ALL   = 0xff
};


// this redefinition enables bitwise operator usage
constexpr bool enable_bitmask_operators(RelayID) { return true; }


// conversion of scoped enum to underlying_type
template<typename E>
constexpr typename std::enable_if<std::is_enum<E>::value, std::underlying_type<E>>::type::type as_number(const E e)
{
    return static_cast<typename std::underlying_type<E>::type>(e);
}


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
    static const quint16 kProductID;
    static const quint16 kVendorID;

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
    static void hexToByte(unsigned char **pbuffer, int *n, const QString &msg);
    static QString byteToHex(const unsigned char *buffer, int n);
    static QString checkSum(const QString &msg);
    static unsigned char checkSum(const unsigned char *bMsg, int n);
    static bool validateResponse(const QString &msg, K8090Traits::Command cmd);
    static bool validateResponse(const unsigned char *bMsg, int n, K8090Traits::Command cmd);

    void sendCommand();

    void sendSwitchRelayOnCommand();
    void sendSwitchRelayOffCommand();

    QString com_port_name_;
    QSerialPort *serial_port_;

    bool commandBuffer_[K8090Traits::as_number(K8090Traits::Command::NONE)];

    K8090Traits::Command last_command_;

    static const unsigned char* commands_;
};

}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_K8090_H_
