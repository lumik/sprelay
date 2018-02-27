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

#include "centralwidget.h"

#include <QComboBox>
#include <QDebug>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QStringBuilder>

#include "indicatorbutton.h"
#include "k8090.h"


namespace sprelay {
namespace gui {

CentralWidget::CentralWidget(core::K8090 *k8090, const QString &com_port_name, QWidget *parent)
    : QWidget{parent}, com_port_name_{com_port_name}
{
    if (k8090) {
        k8090_ = k8090;
    } else {
        k8090_ = new core::K8090(this);
    }

    constructGui();
}

CentralWidget::~CentralWidget()
{
}

void CentralWidget::onConnectButtonClicked()
{
    k8090_->setComPortName(ports_combo_box_->currentText());
    k8090_->connectK8090();
    connect_button_->setState(!connect_button_->state());
}

void CentralWidget::onPortsComboBoxCurrentIndexChanged(const QString &portName)
{
    qDebug() << portName;
}

void CentralWidget::onRefreshPortsButtonClicked()
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

void CentralWidget::constructGui()
{
    createUiElements();
    connectGui();
    makeLayout();
}

void CentralWidget::createUiElements()
{
    connect_button_ = new IndicatorButton(tr("Connect"), this);
    refresh_ports_button_ = new QPushButton(tr("Refresh Ports"), this);
    ports_label_ = new QLabel(tr("Select port:"), this);
    ports_combo_box_ = new QComboBox(this);
    ports_label_->setBuddy(ports_combo_box_);  // buddy accepts focus instead of label (for editing)
    initializePortsCombobox();
}

void CentralWidget::initializePortsCombobox()
{
    int index = 0;
    QList<core::K8090Traits::ComPortParams> com_port_params_list = core::K8090::availablePorts();
    bool current_port_found = false;
    // fill combo box
    for (const core::K8090Traits::ComPortParams &com_port_params : com_port_params_list) {
        ports_combo_box_->insertItem(index++, com_port_params.portName);
        if (com_port_name_ == com_port_params.portName) {
            current_port_found = true;
        }
    }

    // if com_port_name_ doesn't match any port, try to find card.
    if (!current_port_found) {
        for (const core::K8090Traits::ComPortParams &com_port_params : com_port_params_list) {
            if (com_port_params.productIdentifier == core::K8090::kProductID
                    && com_port_params.vendorIdentifier == core::K8090::kVendorID) {
                com_port_name_ = com_port_params.portName;
                break;
            }
        }
    }
    if ((index = ports_combo_box_->findText(com_port_name_)) != -1)
        ports_combo_box_->setCurrentIndex(index);
    com_port_name_ = ports_combo_box_->currentText();
}

void CentralWidget::connectGui()
{
    connect(connect_button_, &QPushButton::clicked, this, &CentralWidget::onConnectButtonClicked);
    connect(refresh_ports_button_, &QPushButton::clicked, this, &CentralWidget::onRefreshPortsButtonClicked);
    connect(ports_combo_box_, static_cast<void (QComboBox::*)(const QString &)>(&QComboBox::currentIndexChanged),
            this, &CentralWidget::onPortsComboBoxCurrentIndexChanged);
}

void CentralWidget::makeLayout()
{
    QHBoxLayout *main_layout = new QHBoxLayout;
    setLayout(main_layout);

    QVBoxLayout *port_v_layout = new QVBoxLayout;
    main_layout->addLayout(port_v_layout);
    port_v_layout->addWidget(connect_button_);
    port_v_layout->addWidget(ports_label_);
    port_v_layout->addWidget(ports_combo_box_);
    port_v_layout->addWidget(refresh_ports_button_);
    port_v_layout->addStretch();

    main_layout->addStretch();
}

}  // namespace gui
}  // namespace sprelay
