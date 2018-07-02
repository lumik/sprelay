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

#ifndef SPRELAY_GUI_CENTRAL_WIDGET_H_
#define SPRELAY_GUI_CENTRAL_WIDGET_H_

#include <QString>
#include <QWidget>

#include <memory>

#include "k8090.h"

// forward declarations
class QComboBox;
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

class CentralWidget : public QWidget
{
    Q_OBJECT
public:  // NOLINT(whitespace/indent)
    explicit CentralWidget(
            core::K8090 *k8090 = nullptr,
            const QString &com_port_name = QString(),
            QWidget *parent = 0);
    ~CentralWidget() override;

signals:  // NOLINT(whitespace/indent)

public slots:  // NOLINT(whitespace/indent)

private slots:  // NOLINT(whitespace/indent)
    // reactions to user interaction with gui
    void onConnectButtonClicked();
    void onPortsComboBoxCurrentIndexChanged(const QString &portName);
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
    void onRelayStatus(sprelay::core::K8090Traits::RelayID previous, sprelay::core::K8090Traits::RelayID current,
        sprelay::core::K8090Traits::RelayID timed);
    void onButtonStatus(sprelay::core::K8090Traits::RelayID state, sprelay::core::K8090Traits::RelayID pressed,
        sprelay::core::K8090Traits::RelayID released);
    void onTotalTimerDelay(sprelay::core::K8090Traits::RelayID relay, quint16 delay);
    void onRemainingTimerDelay(sprelay::core::K8090Traits::RelayID relay, quint16 delay);
    void onButtonModes(sprelay::core::K8090Traits::RelayID momentary, sprelay::core::K8090Traits::RelayID toggle,
       sprelay::core::K8090Traits::RelayID timed);
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

    core::K8090 *k8090_;
    QString com_port_name_;
    bool connected_;

    // GUI elements
    // port settings
    std::unique_ptr<IndicatorButton> connect_button_;
    std::unique_ptr<QPushButton> refresh_ports_button_;
    std::unique_ptr<QComboBox> ports_combo_box_;

    // relays
    // relays globals
    std::unique_ptr<QPushButton> refresh_relays_button_;
    std::unique_ptr<QPushButton> reset_factory_defaults_button_;
    std::unique_ptr<QLabel> firmware_version_label_;
    std::unique_ptr<IndicatorLight> jumper_status_light_;
    // relay button status
    std::unique_ptr<IndicatorLight> pushed_indicators_arr_[kNRelays];
    // power settings
    std::unique_ptr<IndicatorButton> relay_on_buttons_arr_[kNRelays];
    std::unique_ptr<QPushButton> relay_off_buttons_arr_[kNRelays];
    std::unique_ptr<QPushButton> toggle_relay_buttons_arr_[kNRelays];
    // mode settings
    std::unique_ptr<IndicatorButton> momentary_buttons_arr_[kNRelays];
    std::unique_ptr<IndicatorButton> toggle_mode_buttons_arr_[kNRelays];
    std::unique_ptr<IndicatorButton> timed_buttons_arr_[kNRelays];
    // timer settings
    std::unique_ptr<QLabel> default_timer_labels_arr_[kNRelays];
    std::unique_ptr<QLabel> remaining_time_labels_arr_[kNRelays];
    std::unique_ptr<QPushButton> set_default_timer_buttons_arr_[kNRelays];
    std::unique_ptr<IndicatorButton> start_timer_buttons_arr_[kNRelays];
    std::unique_ptr<QSpinBox> timer_spin_box_arr_[kNRelays];

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
