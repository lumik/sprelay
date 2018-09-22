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
 * \file      indicator_button.cpp
 * \brief     Indicator button and indicator light widgets which indicates their state by their color.

 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-06-26
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#include "indicator_button.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSize>
#include <QString>

namespace sprelay {
namespace gui {

// TODO(lumik): improve documentation with examples.
/*!
 * \class IndicatorLight
 * \remarks reentrant
 */


/*!
 * \brief Constructs the widget.
 * \param parent The widget's parent object in Qt ownership system.
 *
 * Sets the widget to false IndicatorLight::state and sets the light to red color.
 */
IndicatorLight::IndicatorLight(QWidget *parent) : QPushButton(parent), state_{false}
{
    QSize indicatorSize(10, 10);
    setFixedSize(indicatorSize);
    setEnabled(false);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    if (state_) {
        setStyleSheet("background-color: green");
    } else {
        setStyleSheet("background-color: red");
    }
}

/*!
 * \property IndicatorLight::state
 * \brief This property maintains the state of indicator light.
 *
 * If the state is set true, the background color is green, otherwise the background color is red. When the state
 * changes, IndicatorLight::stateChanged() signal is emited.
 *
 * | Access functions: ||
 * | - | - |
 * | **Access** | %IndicatorLight::state()       |
 * | **Set**    | IndicatorLight::setState()     |
 * | **Notify** | IndicatorLight::stateChanged() |
 */


/*!
 * \brief Setter for IndicatorLight::state property.
 * \param state The required state.
 */
void IndicatorLight::setState(bool state)
{
    if (state != state_) {
        state_ = state;
        if (state_) {
            setStyleSheet("background-color: green");
        } else {
            setStyleSheet("background-color: red");
        }
        emit stateChanged();
    }
}


/*!
 * \fn IndicatorLight::stateChanged
 * \brief Signal which is emited each time the state changes.
 */


/*!
 * \class IndicatorButton
 * \remarks reentrant
 */


/*!
 * \brief Constructs the widget.
 * \param parent The widget's parent object in Qt ownership system.
 */
IndicatorButton::IndicatorButton(QWidget *parent) : QPushButton{parent}
{
    initialize("");
}


/*!
 * \brief Constructs the widget.
 * \param text The text displayed on the pushbutton.
 * \param parent The widget's parent object in Qt ownership system.
 */
IndicatorButton::IndicatorButton(const QString &text, QWidget *parent) : QPushButton{"", parent}
{
    initialize(text);
}


/*!
 * \brief Constructs the widget.
 * \param icon The icon displayed on the pushbutton. See QPushButton documentation.
 * \param text The text displayed on the pushbutton.
 * \param parent The widget's parent object in Qt ownership system.
 */
IndicatorButton::IndicatorButton(const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton{icon, "", parent}
{
    initialize(text);
}


/*!
 * \property IndicatorButton::state
 * \brief This property maintains the state of indicator button.
 *
 * If the state is set true, the indicator light color is green, otherwise the color is red. When the state changes,
 * IndicatorButton::stateChanged() signal is emited.
 *
 * | Access functions: ||
 * | - | - |
 * | **Access** | bool %IndicatorButton::state()        |
 * | **Set**    | IndicatorButton::setState(bool state) |
 * | **Notify** | IndicatorButton::stateChanged()       |
 */


/*!
 * \property IndicatorButton::text
 * \brief This property contains the text displayed on push button.
 *
 * | Access functions: ||
 * | - | - |
 * | **Access** | QString %IndicatorButton::text()   |
 * | **Set**    | IndicatorButton::setText(const QString &text) |
 */


/*!
 * \brief Setter for the IndicatorButton::text property.
 * \param text The text.
 */
void IndicatorButton::setText(const QString &text)
{
    label_->setText(text);
}


QString IndicatorButton::text() const
{
    return label_->text();
}


/*!
 * \brief Accessor for the inherited QWidget::sizeHint property.
 * \return The size hint.
 */
QSize IndicatorButton::sizeHint() const
{
    return layout()->sizeHint();
}


/*!
 * \fn IndicatorButton::setState(bool state)
 * \brief Setter for IndicatorLight::state property.
 * \param state The required state.
 */

/*!
 * \fn IndicatorButton::stateChanged
 * \brief Signal which is emited each time the state changes.
 */


void IndicatorButton::initialize(const QString &text)
{
    QHBoxLayout *layout = new QHBoxLayout{this};
    indicator_ = new IndicatorLight{this};
    label_ = new QLabel{text, this};

    indicator_->setAttribute(Qt::WA_TransparentForMouseEvents);

    label_->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    label_->setTextInteractionFlags(Qt::NoTextInteraction);
    label_->setMouseTracking(false);
    label_->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    layout->addWidget(indicator_);
    layout->setContentsMargins(5, 5, 5, 5);

    layout->addWidget(label_);
    layout->setSpacing(5);
    layout->setMargin(0);
    layout->setContentsMargins(5, 5, 5, 5);

    setLayout(layout);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);

    connect(indicator_, &IndicatorLight::stateChanged, this, &IndicatorButton::stateChanged);
}

}  // namespace gui
}  // namespace sprelay
