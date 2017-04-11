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

#include "mainwindow.h"

#include <QPushButton>
#include <QLabel>
#include <QComboBox>
#include <QLayout>
#include <QMessageBox>
#include <QStringBuilder>

#include <QDebug>

#include "k8090.h"

MainWindow::MainWindow()
{
    connected_ = false;

    cWidget = new QWidget;

    connectButton = new QPushButton(tr("Connect"));
    setCentralWidget(cWidget);

    qDebug() << "Ahoj";

    k8090 = new K8090(this);

    refreshPortsButton = new QPushButton(tr("Refresh Ports"));
    portsLabel = new QLabel(tr("Select port:"));
    portsComboBox = new QComboBox();
    portsLabel->setBuddy(portsComboBox);  // buddy accepts focus instead of label (for editing)
    int index = 0;
//    refreshingPortsComboBoxContent = true;
    foreach (const K8090Traits::ComPortParams &comPortParams, K8090::availablePorts()) {  // NOLINT(whitespace/parens)
        portsComboBox->insertItem(index++, comPortParams.portName);
    }
    if (connected_) {
//        comPortName_ = k8090->comPortName();
    }
    if ((index = portsComboBox->findText(comPortName_)) != -1)
        portsComboBox->setCurrentIndex(index);
//    refreshingPortsComboBoxContent = false;
    if (!connected_ || index == -1) {
//        k8090->setComPortName(portsComboBox->currentText());
    }
    comPortName_ = portsComboBox->currentText();

    connect(connectButton, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(refreshPortsButton, &QPushButton::clicked, this, &MainWindow::onRefreshPortsButtonClicked);
    connect(portsComboBox, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPortsComboBoxCurrentIndexChanged);

    mainHLayout = new QHBoxLayout;
    cWidget->setLayout(mainHLayout);

    vLayout1 = new QVBoxLayout;
    mainHLayout->addLayout(vLayout1);
    vLayout1->addWidget(connectButton);
    vLayout1->addStretch();

    vLayout2 = new QVBoxLayout;
    mainHLayout->addLayout(vLayout2);

    refreshPortsHLayout = new QHBoxLayout;
    vLayout2->addLayout(refreshPortsHLayout);
    refreshPortsHLayout->addStretch();
    refreshPortsHLayout->addWidget(refreshPortsButton);
    refreshPortsHLayout->addStretch();

    portsHLayout = new QHBoxLayout;
    vLayout2->addLayout(portsHLayout);
    portsHLayout->addWidget(portsLabel);
    portsHLayout->addWidget(portsComboBox);
    portsHLayout->addStretch();

    vLayout2->addStretch();
}

void MainWindow::onConnectButtonClicked()
{
    k8090->connectK8090();
}


void MainWindow::onPortsComboBoxCurrentIndexChanged(const QString &portName)
{
    qDebug() << portName;
}

void MainWindow::onRefreshPortsButtonClicked()
{
    if (!k8090->availablePorts().isEmpty()) {
        QString msg;
        QString currPort = portsComboBox->currentText();
        QStringList comPortNames;
        foreach (const K8090Traits::ComPortParams &comPortParams, K8090::availablePorts()) {  // NOLINT
            msg.append("Port name: " % comPortParams.portName % "\n" %
                       "Description: " % comPortParams.description % "\n" %
                       "Manufacturer: " % comPortParams.manufacturer % "\n" %
                       "Product Identifier: " % QString::number(comPortParams.productIdentifier) % "\n" %
                       "Vendor Identifier: " % QString::number(comPortParams.vendorIdentifier) % "\n");
            comPortNames.append(comPortParams.portName);
        }
        QMessageBox::information(this, tr("Serial ports information:"), msg, QMessageBox::Ok);
        bool ok = true;
        bool counteq = false;
        if ((counteq = (portsComboBox->count() == comPortNames.count()))) {
            for (int ii = 0; ii < comPortNames.count(); ++ii) {
                if (portsComboBox->findText(comPortNames.at(ii)) < 0) {
                    ok = false;
                }
            }
        }
        if (!counteq || !ok) {
//            refreshingPortsComboBoxContent = true;
            portsComboBox->clear();
            portsComboBox->insertItems(1, comPortNames);
            int index;
            if ((index = portsComboBox->findText(currPort)) >= 0)
                portsComboBox->setCurrentIndex(index);
            else if ((index = portsComboBox->findText(comPortName_)) >= 0)
                portsComboBox->setCurrentIndex(index);
//            refreshingPortsComboBoxContent = false;
            if (!connected_ && index == -1) {
//                neslab->setComPortName(portsComboBox->currentText());
            }
            comPortName_ = portsComboBox->currentText();
        }
    } else {
//        onDisconnected();
//        onNoSerialPortAvailable();
    }
}


MainWindow::~MainWindow()
{
}
