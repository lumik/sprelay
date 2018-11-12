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
 * \file      indicator_button.h
 * \brief     The biomolecules::sprelay::gui::IndicatorButton and biomolecules::sprelay::gui::IndicatorLight widgets
 *            which indicates their state by their color.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-06-26
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_GUI_INDICATOR_BUTTON_H_
#define BIOMOLECULES_SPRELAY_GUI_INDICATOR_BUTTON_H_

#include <QPushButton>

// forward declarations
class QLabel;

namespace biomolecules {
namespace sprelay {
namespace gui {

/// \brief The class defining widget which can be used as two state indicator light.
/// \headerfile ""
class IndicatorLight : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool state MEMBER state_ READ state WRITE setState NOTIFY stateChanged)

public:
    explicit IndicatorLight(QWidget* parent = nullptr);
    IndicatorLight(const IndicatorLight&) = delete;
    IndicatorLight& operator=(const IndicatorLight&) = delete;

    /// Getter for IndicatorLight::state property.
    bool state() const { return state_; }

public slots:
    void setState(bool state);

signals:
    void stateChanged();

private:
    bool state_;
};


/// \brief The class defining widget which can be used as two state button with indicator light.
/// \headerfile ""
class IndicatorButton : public QPushButton
{
    Q_OBJECT
    Q_PROPERTY(bool state READ state WRITE setState NOTIFY stateChanged)
    Q_PROPERTY(QString text READ text WRITE setText)

public:
    explicit IndicatorButton(QWidget* parent = nullptr);
    explicit IndicatorButton(const QString& text, QWidget* parent = nullptr);
    IndicatorButton(const QIcon& icon, const QString& text, QWidget* parent = nullptr);
    IndicatorButton(const IndicatorButton&) = delete;
    IndicatorButton& operator=(const IndicatorButton&) = delete;

    void setText(const QString& text);
    /// Getter for IndicatorButton::text property.
    QString text() const;
    QSize sizeHint() const override;
    /// Getter for IndicatorButton::state property.
    bool state() const { return indicator_->state(); }

public slots:
    void setState(bool state) { indicator_->setState(state); }

signals:
    void stateChanged();

private:
    void initialize(const QString& text);
    QLabel* label_;
    IndicatorLight* indicator_;
};

}  // namespace gui
}  // namespace sprelay
}  // namespace biomolecules

#endif  // BIOMOLECULES_SPRELAY_GUI_INDICATOR_BUTTON_H_
