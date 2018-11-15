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
 * \file      central_widget.h
 * \ingroup   group_biomolecules_sprelay_gui_public
 * \brief     The biomolecules::sprelay::gui::CentralWidget class which provides means to control the relay card
 *            through GUI.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-03-07
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_GUI_CENTRAL_WIDGET_H_
#define BIOMOLECULES_SPRELAY_GUI_CENTRAL_WIDGET_H_

#include <array>
#include <memory>

#include <QString>
#include <QWidget>

#include "biomolecules/sprelay/core/k8090.h"
#include "biomolecules/sprelay/sprelay_global.h"

// forward declarations
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSignalMapper;
class QSpinBox;
class QTimer;

namespace biomolecules {
namespace sprelay {
namespace core {
// forward declarations of serial_utils classes
namespace serial_utils {
struct ComPortParams;
}  // namespace serial_utils
}  // namespace core
namespace gui {
class IndicatorButton;
class IndicatorLight;

/// Widget which controlls Velleman %K8090 card.
class SPRELAY_EXPORT CentralWidget : public QWidget
{
    Q_OBJECT
public:
    explicit CentralWidget(core::k8090::K8090* k8090 = nullptr,
        QString com_port_name = QString{},
        QWidget* parent = nullptr);
    CentralWidget(const CentralWidget&) = delete;
    CentralWidget(CentralWidget&&) = delete;
    CentralWidget& operator=(const CentralWidget&) = delete;
    CentralWidget& operator=(CentralWidget&&) = delete;
    ~CentralWidget() override;

signals:

public slots:

private slots:
    // reactions to user interaction with gui
    void onConnectButtonClicked();
    void onPortsComboBoxCurrentIndexChanged(const QString& port_name);
    void onRefreshPortsButtonClicked();
    void onRefreshRelaysButtonClicked();
    void resetFactoryDefaultsButtonClicked();
    void onRelayOnButtonClicked(int relay);
    void onRelayOffButtonClicked(int relay);
    void onToggleRelayButtonClicked(int relay);
    void onMomentaryButtonClicked(int relay);
    void onToggleModeButtonClicked(int relay);
    void onTimedButtonClicked(int relay);
    void onSetDefaultTimerButtonClicked(int relay);
    void onStartTimerButtonClicked(int relay);
    void onTimerSpinBoxValueChanged(int relay);

private slots:
    // reactions to signals from the relay
    void onRelayStatus(biomolecules::sprelay::core::k8090::RelayID previous,
        biomolecules::sprelay::core::k8090::RelayID current, biomolecules::sprelay::core::k8090::RelayID timed);
    void onButtonStatus(biomolecules::sprelay::core::k8090::RelayID state,
        biomolecules::sprelay::core::k8090::RelayID pressed, biomolecules::sprelay::core::k8090::RelayID released);
    void onTotalTimerDelay(biomolecules::sprelay::core::k8090::RelayID relay, quint16 delay);
    void onRemainingTimerDelay(biomolecules::sprelay::core::k8090::RelayID relay, quint16 delay);
    void onButtonModes(biomolecules::sprelay::core::k8090::RelayID momentary,
        biomolecules::sprelay::core::k8090::RelayID toggle, biomolecules::sprelay::core::k8090::RelayID timed);
    void onJumperStatus(bool on);
    void onFirmwareVersion(int year, int week);
    void onConnected();
    void onConnectionFailed();
    void onNotConnected();
    void onDisconnected();

    // other slots
    void onRefreshTimersDelay();


private:
    static const int kNRelays = 8;
    static const int kRefreshTimersRateMs_ = 300;
    void constructGui();
    void createUiElements();
    void initializePortsCombobox();
    void connectGui();
    void makeLayout();
    void connectionStatusChanged();

    core::k8090::K8090* k8090_;
    QString com_port_name_;
    bool connected_;

    // GUI elements
    // port settings
    IndicatorButton* connect_button_;
    QPushButton* refresh_ports_button_;
    QComboBox* ports_combo_box_;

    // relays
    // relays globals
    QGroupBox* relays_globals_box_;
    QPushButton* refresh_relays_button_;
    QPushButton* reset_factory_defaults_button_;
    QLabel* firmware_version_label_;
    IndicatorLight* jumper_status_light_;
    // relay button status
    QGroupBox* relay_button_status_settings_box_;
    std::array<IndicatorLight*, kNRelays> pushed_indicators_arr_;
    // power settings
    QGroupBox* relay_power_settings_box_;
    std::array<IndicatorButton*, kNRelays> relay_on_buttons_arr_;
    std::array<QPushButton*, kNRelays> relay_off_buttons_arr_;
    std::array<QPushButton*, kNRelays> toggle_relay_buttons_arr_;
    // mode settings
    QGroupBox* relay_mode_settings_box_;
    std::array<IndicatorButton*, kNRelays> momentary_buttons_arr_;
    std::array<IndicatorButton*, kNRelays> toggle_mode_buttons_arr_;
    std::array<IndicatorButton*, kNRelays> timed_buttons_arr_;
    // timer settings
    QGroupBox* relay_timers_settings_box_;
    std::array<QLabel*, kNRelays> default_timer_labels_arr_;
    std::array<QLabel*, kNRelays> remaining_time_labels_arr_;
    std::array<QPushButton*, kNRelays> set_default_timer_buttons_arr_;
    std::array<IndicatorButton*, kNRelays> start_timer_buttons_arr_;
    std::array<QSpinBox*, kNRelays> timer_spin_box_arr_;

    // signal mappers
    std::unique_ptr<QSignalMapper> relay_on_mapper_;
    std::unique_ptr<QSignalMapper> relay_off_mapper_;
    std::unique_ptr<QSignalMapper> toggle_relay_mapper_;
    std::unique_ptr<QSignalMapper> momentary_mapper_;
    std::unique_ptr<QSignalMapper> timed_mapper_;
    std::unique_ptr<QSignalMapper> toggle_mode_mapper_;
    std::unique_ptr<QSignalMapper> set_default_timer_mapper_;
    std::unique_ptr<QSignalMapper> start_timer_mapper_;
    std::unique_ptr<QSignalMapper> timer_spin_box_mapper_;

    std::unique_ptr<QTimer> refresh_delay_timer_;
};

}  // namespace gui
}  // namespace sprelay
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPRELAY_GUI_CENTRAL_WIDGET_H_
