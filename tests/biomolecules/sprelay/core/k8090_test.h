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
 * \file      k8090_test.h
 * \brief     The biomolecules::sprelay::core::k8090::K8090Test class which implements tests for
 *            biomolecules::sprelay::core::k8090::K8090.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2017-03-21
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_CORE_K8090_TEST_H_
#define BIOMOLECULES_SPRELAY_CORE_K8090_TEST_H_

#include <QObject>
#include <QString>

#include <memory>

#include "lumik/qtest_suite/qtest_suite.h"

#include "biomolecules/sprelay/core/k8090.h"

// forward declarations
class QSignalSpy;

namespace biomolecules {
namespace sprelay {
namespace core {
namespace k8090 {

// forward declarations
class K8090;

class K8090Test : public QObject
{
    Q_OBJECT
private slots:
    void initTestCase();
    void init();
    void cleanup();
    void connectK8090_data();
    void connectK8090();
    void disconnect_data();
    void disconnect();
    void refreshRelaysInfo_data();
    void refreshRelaysInfo();
    void switchRelayOnOff_data();
    void switchRelayOnOff();
    void toggleRelay_data();
    void toggleRelay();
    void buttonMode_data();
    void buttonMode();
    void totalTimer_data();
    void totalTimer();
    void startTimer_data();
    void startTimer();
    void factoryDefaults_data();
    void factoryDefaults();
    void jumperStatus_data();
    void jumperStatus();
    void firmwareVersion_data();
    void firmwareVersion();
    void priorities_data();
    void priorities();

private:
    void createTestData();
    bool checkNoSpyData(QSignalSpy** spies, int n);

    std::unique_ptr<K8090> k8090_;
    bool real_card_present_;
    QString real_card_port_name_;
};

// NOLINTNEXTLINE(cert-err58-cpp)
ADD_TEST(K8090Test)

}  // namespace k8090
}  // namespace core
}  // namespace sprelay
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPRELAY_CORE_K8090_TEST_H_
