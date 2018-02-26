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

#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStringBuilder>

#include "k8090.h"


namespace sprelay {
namespace gui {

MainWindow::MainWindow()
{
    connected_ = false;

    central_widget_ = new QWidget(this);

    connect_button_ = new IndicatorButton(tr("Connect"), this);
    setCentralWidget(central_widget_);

    k8090_ = new core::K8090(this);

    refresh_ports_button_ = new QPushButton(tr("Refresh Ports"), this);
    ports_label_ = new QLabel(tr("Select port:"), this);
    ports_combo_box_ = new QComboBox(this);
    ports_label_->setBuddy(ports_combo_box_);  // buddy accepts focus instead of label (for editing)
    int index = 0;
//    refreshingPortsComboBoxContent = true;
    foreach (const core::K8090Traits::ComPortParams &comPortParams,  // NOLINT(whitespace/parens)
             core::K8090::availablePorts()) {
        ports_combo_box_->insertItem(index++, comPortParams.portName);
    }
    if (connected_) {
//        comPortName_ = core::k8090->comPortName();
    }
    if ((index = ports_combo_box_->findText(com_port_name_)) != -1)
        ports_combo_box_->setCurrentIndex(index);
//    refreshingPortsComboBoxContent = false;
    if (!connected_ || index == -1) {
//        core::k8090->setComPortName(portsComboBox->currentText());
    }
    com_port_name_ = ports_combo_box_->currentText();

    connect(connect_button_, &QPushButton::clicked, this, &MainWindow::onConnectButtonClicked);
    connect(refresh_ports_button_, &QPushButton::clicked, this, &MainWindow::onRefreshPortsButtonClicked);
    connect(ports_combo_box_, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &MainWindow::onPortsComboBoxCurrentIndexChanged);

    main_layout_ = new QHBoxLayout;
    central_widget_->setLayout(main_layout_);

    v_layout_1 = new QVBoxLayout;
    main_layout_->addLayout(v_layout_1);
    v_layout_1->addWidget(connect_button_);
    v_layout_1->addStretch();

    v_layout_2 = new QVBoxLayout;
    main_layout_->addLayout(v_layout_2);

    refresh_ports_h_layout_ = new QHBoxLayout;
    v_layout_2->addLayout(refresh_ports_h_layout_);
    refresh_ports_h_layout_->addStretch();
    refresh_ports_h_layout_->addWidget(refresh_ports_button_);
    refresh_ports_h_layout_->addStretch();

    ports_h_layout_ = new QHBoxLayout;
    v_layout_2->addLayout(ports_h_layout_);
    ports_h_layout_->addWidget(ports_label_);
    ports_h_layout_->addWidget(ports_combo_box_);
    ports_h_layout_->addStretch();

    v_layout_2->addStretch();
}

void MainWindow::onConnectButtonClicked()
{
    k8090_->connectK8090();
    connect_button_->setState(!connect_button_->state());
}


void MainWindow::onPortsComboBoxCurrentIndexChanged(const QString &portName)
{
    qDebug() << portName;
}

void MainWindow::onRefreshPortsButtonClicked()
{
    if (!k8090_->availablePorts().isEmpty()) {
        QString msg;
        QString currPort = ports_combo_box_->currentText();
        QStringList comPortNames;
        foreach (const core::K8090Traits::ComPortParams &comPortParams,  // NOLINT(whitespace/parens)
                 core::K8090::availablePorts()) {
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
        if ((counteq = (ports_combo_box_->count() == comPortNames.count()))) {
            for (int ii = 0; ii < comPortNames.count(); ++ii) {
                if (ports_combo_box_->findText(comPortNames.at(ii)) < 0) {
                    ok = false;
                }
            }
        }
        if (!counteq || !ok) {
//            refreshingPortsComboBoxContent = true;
            ports_combo_box_->clear();
            ports_combo_box_->insertItems(1, comPortNames);
            int index;
            if ((index = ports_combo_box_->findText(currPort)) >= 0)
                ports_combo_box_->setCurrentIndex(index);
            else if ((index = ports_combo_box_->findText(com_port_name_)) >= 0)
                ports_combo_box_->setCurrentIndex(index);
//            refreshingPortsComboBoxContent = false;
            if (!connected_ && index == -1) {
//                neslab->setComPortName(portsComboBox->currentText());
            }
            com_port_name_ = ports_combo_box_->currentText();
        }
    } else {
//        onDisconnected();
//        onNoSerialPortAvailable();
    }
}


MainWindow::~MainWindow()
{
}

}  // namespace gui
}  // namespace sprelay
