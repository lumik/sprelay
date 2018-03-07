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

#include "indicator_button.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSize>
#include <QString>

namespace sprelay {
namespace gui {


IndicatorLight::IndicatorLight(QWidget *parent) : QPushButton(parent)
{
    QSize indicatorSize(10, 10);
    setFixedSize(indicatorSize);
    setEnabled(false);
    setAttribute(Qt::WA_TransparentForMouseEvents);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}


IndicatorButton::IndicatorButton(QWidget *parent) : QPushButton{parent}, state_{false}
{
    initialize("");
}


IndicatorButton::IndicatorButton(const QString &text, QWidget *parent) : QPushButton{"", parent}, state_{false}
{
    initialize(text);
}


IndicatorButton::IndicatorButton(const QIcon &icon, const QString &text, QWidget *parent)
    : QPushButton{icon, "", parent}, state_{false}
{
    initialize(text);
}


void IndicatorButton::setText(const QString &text)
{
    label_->setText(text);
}


QString IndicatorButton::text() const
{
    return label_->text();
}


QSize IndicatorButton::sizeHint() const
{
    return layout()->sizeHint();
}


void IndicatorButton::setState(bool state)
{
    if (state != state_) {
        state_ = state;
        if (state_) {
            indicator_->setStyleSheet("background-color: green");
        } else {
            indicator_->setStyleSheet("background-color: red");
        }
        emit stateChanged();
    }
}


void IndicatorButton::initialize(const QString &text)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    indicator_ = new IndicatorLight(this);
    label_ = new QLabel(text, this);

    if (state_) {
        indicator_->setStyleSheet("background-color: green");
    } else {
        indicator_->setStyleSheet("background-color: red");
    }

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
}

}  // namespace gui
}  // namespace sprelay
