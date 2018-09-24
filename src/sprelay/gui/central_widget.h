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
 * \ingroup   group_sprelay_gui_public
 * \brief     The sprelay::gui::CentralWidget class which provides means to control the relay card through GUI.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-03-07
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_GUI_CENTRAL_WIDGET_H_
#define SPRELAY_GUI_CENTRAL_WIDGET_H_

#include <QString>
#include <QWidget>

#include <memory>

#include "sprelay/sprelay_global.h"
#include "sprelay/core/k8090.h"

// forward declarations
class QComboBox;
class QGroupBox;
class QLabel;
class QPushButton;
class QSignalMapper;
class QSpinBox;
class QTimer;

namespace sprelay {
namespace core {
// forward declarations of serial_utils classes
namespace serial_utils {
class ComPortParams;
}
}
namespace gui {
class IndicatorButton;
class IndicatorLight;

/// Widget which controlls Velleman %K8090 card.
class SPRELAY_EXPORT CentralWidget : public QWidget
{
    Q_OBJECT
public:  // NOLINT(whitespace/indent)
    explicit CentralWidget(
            core::k8090::K8090 *k8090 = nullptr,
            const QString &com_port_name = QString{},
            QWidget *parent = 0);
    ~CentralWidget() override;

signals:  // NOLINT(whitespace/indent)

public slots:  // NOLINT(whitespace/indent)

private slots:  // NOLINT(whitespace/indent)
    // reactions to user interaction with gui
    void onConnectButtonClicked();
    void onPortsComboBoxCurrentIndexChanged(const QString &port_name);
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

private slots:  // NOLINT(whitespace/indent)
    // reactions to signals from the relay
    void onRelayStatus(sprelay::core::k8090::RelayID previous, sprelay::core::k8090::RelayID current,
        sprelay::core::k8090::RelayID timed);
    void onButtonStatus(sprelay::core::k8090::RelayID state, sprelay::core::k8090::RelayID pressed,
        sprelay::core::k8090::RelayID released);
    void onTotalTimerDelay(sprelay::core::k8090::RelayID relay, quint16 delay);
    void onRemainingTimerDelay(sprelay::core::k8090::RelayID relay, quint16 delay);
    void onButtonModes(sprelay::core::k8090::RelayID momentary, sprelay::core::k8090::RelayID toggle,
       sprelay::core::k8090::RelayID timed);
    void onJumperStatus(bool on);
    void onFirmwareVersion(int year, int week);
    void onConnected();
    void onConnectionFailed();
    void onNotConnected();
    void onDisconnected();

    // other slots
    void onRefreshTimersDelay();


private:  // NOLINT(whitespace/indent)
    static const int kNRelays = 8;
    static const int kRefreshTimersRateMs_ = 300;
    void constructGui();
    void createUiElements();
    void initializePortsCombobox();
    void connectGui();
    void makeLayout();
    void connectionStatusChanged();

    core::k8090::K8090 *k8090_;
    QString com_port_name_;
    bool connected_;

    // GUI elements
    // port settings
    IndicatorButton *connect_button_;
    QPushButton *refresh_ports_button_;
    QComboBox *ports_combo_box_;

    // relays
    // relays globals
    QGroupBox *relays_globals_box_;
    QPushButton *refresh_relays_button_;
    QPushButton *reset_factory_defaults_button_;
    QLabel *firmware_version_label_;
    IndicatorLight *jumper_status_light_;
    // relay button status
    QGroupBox *relay_button_status_settings_box_;
    IndicatorLight * pushed_indicators_arr_[kNRelays];
    // power settings
    QGroupBox *relay_power_settings_box_;
    IndicatorButton * relay_on_buttons_arr_[kNRelays];
    QPushButton * relay_off_buttons_arr_[kNRelays];
    QPushButton * toggle_relay_buttons_arr_[kNRelays];
    // mode settings
    QGroupBox *relay_mode_settings_box_;
    IndicatorButton * momentary_buttons_arr_[kNRelays];
    IndicatorButton * toggle_mode_buttons_arr_[kNRelays];
    IndicatorButton * timed_buttons_arr_[kNRelays];
    // timer settings
    QGroupBox *relay_timers_settings_box_;
    QLabel * default_timer_labels_arr_[kNRelays];
    QLabel * remaining_time_labels_arr_[kNRelays];
    QPushButton * set_default_timer_buttons_arr_[kNRelays];
    IndicatorButton * start_timer_buttons_arr_[kNRelays];
    QSpinBox * timer_spin_box_arr_[kNRelays];

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

#endif  // SPRELAY_GUI_CENTRAL_WIDGET_H_
