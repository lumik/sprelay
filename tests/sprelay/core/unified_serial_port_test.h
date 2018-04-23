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

#ifndef SPRELAY_CORE_UNIFIED_SERIAL_PORT_TEST_H_
#define SPRELAY_CORE_UNIFIED_SERIAL_PORT_TEST_H_

#include <QObject>
#include <QString>

#include <memory>

#include "core_test_utils.h"


namespace sprelay {
namespace core {

class UnifiedSerialPort;

class UnifiedSerialPortTest : public QObject
{
    Q_OBJECT
public:  // NOLINT(whitespace/indent)
    static const int kCommandTimeoutMs;
    static const int kDelayBetweenCommandsMs;

private slots:  // NOLINT(whitespace/indent)
    void initTestCase();
    void availablePorts();
    void realBenchmark_data();
    void realBenchmark();
    void realJumperStatus();
    void realFirmwareVersion();
    void realQueryAllTimers();
    void realTimer();
    void realDefaultTimer();

private:  // NOLINT(whitespace/indent)
    unsigned char checkSum(const unsigned char *bMsg, int n);

    std::unique_ptr<UnifiedSerialPort> createSerialPort(QString port_name) const;
    void resetRelays(UnifiedSerialPort *serial_port) const;
    bool compareResponse(const unsigned char *response, const unsigned char *expected);
    void sendCommand(UnifiedSerialPort *serial_port, const unsigned char *command) const;
    bool measureCommandWithResponse(UnifiedSerialPort *serial_port, const unsigned char *message, qint64 *elapsed_ms);
    bool real_card_present_;
    QString real_card_port_name_;
};

}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_UNIFIED_SERIAL_PORT_TEST_H_
