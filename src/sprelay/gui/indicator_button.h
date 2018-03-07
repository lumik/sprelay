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

#ifndef SPRELAY_GUI_INDICATOR_BUTTON_H_
#define SPRELAY_GUI_INDICATOR_BUTTON_H_

#include <QPushButton>

// forward declarations
class QLabel;

namespace sprelay {
namespace gui {

class IndicatorLight : public QPushButton
{
    Q_OBJECT

public:  // NOLINT(whitespace/indent)
    explicit IndicatorLight(QWidget *parent = nullptr);
    IndicatorLight(const IndicatorLight &) = delete;
    IndicatorLight & operator=(const IndicatorLight &) = delete;
};


class IndicatorButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool state MEMBER state_ READ state WRITE setState NOTIFY stateChanged)

public:  // NOLINT(whitespace/indent)
    explicit IndicatorButton(QWidget *parent = nullptr);
    explicit IndicatorButton(const QString &text, QWidget *parent = nullptr);
    IndicatorButton(const QIcon &icon, const QString &text, QWidget *parent = nullptr);
    IndicatorButton(const IndicatorButton &) = delete;
    IndicatorButton & operator=(const IndicatorButton &) = delete;

    void setText(const QString &text);
    QString text() const;
    QSize sizeHint() const;
    bool state() const { return state_; }

public slots:  // NOLINT(whitespace/indent)
    void setState(bool state);

signals:  // NOLINT(whitespace/indent)
    void stateChanged();

private:  // NOLINT(whitespace/indent)
    void initialize(const QString &text);
    QLabel *label_;
    IndicatorLight *indicator_;
    bool state_;
};

}  // namespace gui
}  // namespace sprelay

#endif  // SPRELAY_GUI_INDICATOR_BUTTON_H_
