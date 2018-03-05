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
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QStringBuilder>

#include <limits>
#include <cstdint>

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
            if (!connected_ && index == -1) {
//                k8090->setComPortName(portsComboBox->currentText());
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
    // COM port
    connect_button_ = new IndicatorButton{tr("Connect"), this};
    refresh_ports_button_ = new QPushButton{tr("Refresh Ports"), this};
    ports_combo_box_ = new QComboBox(this);
    initializePortsCombobox();

    // relays
    // globals
    refresh_relays_button_ = new QPushButton{tr("Refresh"), this};
    reset_factory_defaults_button_ = new QPushButton{tr("Factory Reset"), this};
    firmware_version_label_ = new QLabel{tr("Firmware version: 1.0.0"), this};
    jumper_status_light = new IndicatorLight{this};
    for (int i = 0; i < N_relays; ++i) {
        relay_on_buttons_arr_[i] = std::unique_ptr<IndicatorButton>{new IndicatorButton{this}};
        relay_off_buttons_arr_[i] = std::unique_ptr<QPushButton>{new QPushButton{this}};
        toggle_relay_buttons_arr_[i] = std::unique_ptr<QPushButton>{new QPushButton{this}};
        momentary_buttons_arr_[i] = std::unique_ptr<IndicatorButton>{new IndicatorButton{this}};
        timed_buttons_arr_[i] = std::unique_ptr<IndicatorButton>{new IndicatorButton{this}};
        toggle_mode_buttons_arr_[i] = std::unique_ptr<IndicatorButton>{new IndicatorButton{this}};
        default_timer_labels_arr_[i] = std::unique_ptr<QLabel>{new QLabel{"0", this}};
        remaining_time_labels_arr_[i] = std::unique_ptr<QLabel>{new QLabel{"0", this}};
        set_default_timer_buttons_arr_[i] = std::unique_ptr<QPushButton>{new QPushButton{this}};
        timer_spin_box_arr_[i] = std::unique_ptr<QSpinBox>{new QSpinBox{this}};
        timer_spin_box_arr_[i]->setMinimum(0);
        timer_spin_box_arr_[i]->setMaximum(std::numeric_limits<std::uint16_t>::max());
        start_timer_buttons_arr_[i] = std::unique_ptr<IndicatorButton>{new IndicatorButton{this}};
    }
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
    main_layout->setSizeConstraint(QLayout::SetFixedSize);

    // COM port settings
    QVBoxLayout *port_v_layout = new QVBoxLayout;
    main_layout->addLayout(port_v_layout, 0);
    port_v_layout->addWidget(connect_button_);
    QLabel *ports_label = new QLabel(tr("Select port:"), this);
    ports_label->setBuddy(ports_combo_box_);  // buddy accepts focus instead of label (for editing)
    port_v_layout->addWidget(ports_label);
    port_v_layout->addWidget(ports_combo_box_);
    port_v_layout->addWidget(refresh_ports_button_);

    // Relays globals
    QGroupBox *relays_globals_box = new QGroupBox{tr("Relays globals")};
    port_v_layout->addWidget(relays_globals_box);
    QVBoxLayout *relays_globals_layout = new QVBoxLayout;
    relays_globals_box->setLayout(relays_globals_layout);
    relays_globals_layout->addWidget(refresh_relays_button_);
    relays_globals_layout->addWidget(firmware_version_label_);
    QHBoxLayout *jumper_status_layout = new QHBoxLayout;
    relays_globals_layout->addLayout(jumper_status_layout);
    jumper_status_layout->addWidget(new QLabel{tr("Jumper Status:"), this});
    jumper_status_layout->addWidget(jumper_status_light);
    relays_globals_layout->addWidget(reset_factory_defaults_button_);

    port_v_layout->addStretch();

    // relays power settings
    QVBoxLayout *relays_grid_v_layout = new QVBoxLayout;
    main_layout->addLayout(relays_grid_v_layout);
    QGroupBox *relay_power_settings_box = new QGroupBox{tr("Relays power settings")};
    relays_grid_v_layout->addWidget(relay_power_settings_box);
    QGridLayout *power_grid_layout = new QGridLayout;
    relay_power_settings_box->setLayout(power_grid_layout);
    power_grid_layout->addWidget(new QLabel{tr("Relay:"), this}, 0, 0);
    power_grid_layout->addWidget(new QLabel{tr("Switch on:"), this}, 1, 0);
    power_grid_layout->addWidget(new QLabel{tr("Switch off:"), this}, 2, 0);
    power_grid_layout->addWidget(new QLabel{tr("Toggle:"), this}, 3, 0);
    for (int i = 0; i < N_relays; ++i) {
        power_grid_layout->addWidget(new QLabel{QString::number(i + 1), this}, 0, i + 1, Qt::AlignHCenter);
        power_grid_layout->addWidget(relay_on_buttons_arr_[i].get(), 1, i + 1, Qt::AlignHCenter);
        power_grid_layout->addWidget(relay_off_buttons_arr_[i].get(), 2, i + 1, Qt::AlignHCenter);
        power_grid_layout->addWidget(toggle_relay_buttons_arr_[i].get(), 3, i + 1, Qt::AlignHCenter);
    }

    // relays mode settings
    QGroupBox *relay_mode_settings_box = new QGroupBox{tr("Relays mode settings")};
    relays_grid_v_layout->addWidget(relay_mode_settings_box);
    QGridLayout *mode_grid_layout = new QGridLayout;
    relay_mode_settings_box->setLayout(mode_grid_layout);
    mode_grid_layout->addWidget(new QLabel{tr("Relay:"), this}, 0, 0);
    mode_grid_layout->addWidget(new QLabel(tr("Momentary:"), this), 1, 0);
    mode_grid_layout->addWidget(new QLabel(tr("Timed:"), this), 2, 0);
    mode_grid_layout->addWidget(new QLabel(tr("Toggle:"), this), 3, 0);
    for (int i = 0; i < N_relays; ++i) {
        mode_grid_layout->addWidget(new QLabel{QString::number(i + 1), this}, 0, i + 1, Qt::AlignHCenter);
        mode_grid_layout->addWidget(momentary_buttons_arr_[i].get(), 1, i + 1, Qt::AlignHCenter);
        mode_grid_layout->addWidget(timed_buttons_arr_[i].get(), 2, i + 1, Qt::AlignHCenter);
        mode_grid_layout->addWidget(toggle_mode_buttons_arr_[i].get(), 3, i + 1, Qt::AlignHCenter);
    }

    // relay timers settings
    QGroupBox *relay_timers_settings_box = new QGroupBox{tr("Relay timers settings")};
    relays_grid_v_layout->addWidget(relay_timers_settings_box);
    QGridLayout *timer_grid_layout = new QGridLayout;
    relay_timers_settings_box->setLayout(timer_grid_layout);
    timer_grid_layout->addWidget(new QLabel{tr("Relay:"), this}, 0, 0);
    timer_grid_layout->addWidget(new QLabel(tr("Default timer (s):"), this), 1, 0);
    timer_grid_layout->addWidget(new QLabel(tr("Remaining time (s):"), this), 2, 0);
    timer_grid_layout->addWidget(new QLabel(tr("Default:"), this), 3, 0);
    timer_grid_layout->addWidget(new QLabel(tr("Start:"), this), 4, 0);
    timer_grid_layout->addWidget(new QLabel(tr("Timer (s):"), this), 5, 0);
    for (int i = 0; i < N_relays; ++i) {
        timer_grid_layout->addWidget(new QLabel{QString::number(i + 1), this}, 0, i + 1, Qt::AlignHCenter);
        timer_grid_layout->addWidget(default_timer_labels_arr_[i].get(), 1, i + 1, Qt::AlignHCenter);
        timer_grid_layout->addWidget(remaining_time_labels_arr_[i].get(), 2, i + 1, Qt::AlignHCenter);
        timer_grid_layout->addWidget(set_default_timer_buttons_arr_[i].get(), 3, i + 1, Qt::AlignHCenter);
        timer_grid_layout->addWidget(start_timer_buttons_arr_[i].get(), 4, i + 1, Qt::AlignHCenter);
        timer_grid_layout->addWidget(timer_spin_box_arr_[i].get(), 5, i + 1, Qt::AlignHCenter);
    }

    const int layout_no = 3;
    QGridLayout * grid_layouts[layout_no] = {power_grid_layout, mode_grid_layout, timer_grid_layout};
    int relay_label_min_width = std::numeric_limits<int>::min();
    int relay_label_width;
    for (int i = 0; i < layout_no; ++i) {
        for (int j = 0; j < grid_layouts[i]->rowCount(); ++j) {
            relay_label_width = grid_layouts[i]->itemAtPosition(j, 0)->sizeHint().width();
            if (relay_label_width > relay_label_min_width) {
                relay_label_min_width = relay_label_width;
            }
        }
    }
    for (int i = 0; i < layout_no; ++i) {
        grid_layouts[i]->setColumnMinimumWidth(0, relay_label_min_width);
    }

    relays_grid_v_layout->addStretch();
    main_layout->addStretch();
}

}  // namespace gui
}  // namespace sprelay
