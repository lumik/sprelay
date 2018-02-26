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

#ifndef SPRELAY_GUI_MAINWINDOW_H_
#define SPRELAY_GUI_MAINWINDOW_H_

#include <QMainWindow>

// forward declarations
class QComboBox;
class QHBoxLayout;
class QLabel;
class QPushButton;
class QVBoxLayout;


namespace sprelay {
namespace core {
// forward declarations of core classes
class K8090;
}
namespace gui {

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:  // NOLINT(whitespace/indent)
    MainWindow();
    ~MainWindow();


private slots:  // NOLINT(whitespace/indent)
    void onConnectButtonClicked();
    void onPortsComboBoxCurrentIndexChanged(const QString &portName);
    void onRefreshPortsButtonClicked();


private:  // NOLINT(whitespace/indent)
    core::K8090 *k8090_;
    QString com_port_name_;
    bool connected_;

    QWidget *central_widget_;

    QHBoxLayout *main_layout_;

    QVBoxLayout *v_layout_1;
    QPushButton *connect_button_;

    QVBoxLayout *v_layout_2;
    QHBoxLayout *refresh_ports_h_layout_;
    QPushButton *refresh_ports_button_;
    QHBoxLayout *ports_h_layout_;
    QLabel *ports_label_;
    QComboBox *ports_combo_box_;
};

}  // namespace gui
}  // namespace sprelay

#endif  // SPRELAY_GUI_MAINWINDOW_H_
