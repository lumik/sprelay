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

#ifndef GUI_MAINWINDOW_H_
#define GUI_MAINWINDOW_H_

#include <QMainWindow>

class QVBoxLayout;
class QHBoxLayout;
class QPushButton;
class QLabel;
class QComboBox;

class K8090;

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
    K8090 *k8090;
    QString comPortName_;
    bool connected_;

    QWidget *cWidget;

    QHBoxLayout *mainHLayout;

    QVBoxLayout *vLayout1;
    QPushButton *connectButton;

    QVBoxLayout *vLayout2;
    QHBoxLayout *refreshPortsHLayout;
    QPushButton *refreshPortsButton;
    QHBoxLayout *portsHLayout;
    QLabel *portsLabel;
    QComboBox *portsComboBox;
};

#endif  // GUI_MAINWINDOW_H_
