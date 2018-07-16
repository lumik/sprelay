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
 * \file      k8090_utils.h
 * \brief     Utility functions and data structures for K8090 class implementation.

 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-16
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_CORE_K8090_UTILS_H_
#define SPRELAY_CORE_K8090_UTILS_H_

#include "k8090_defines.h"

namespace sprelay {
namespace core {
namespace k8090 {
namespace impl_ {

enum struct TimerDelayType : unsigned char
{
    TOTAL     = 0,
    REMAINING = 1 << 0,
    ALL       = 0xff
};


struct Command
{
    using IdType = k8090::CommandID;
    using NumberType = typename std::underlying_type<IdType>::type;

    Command() : id(k8090::CommandID::NONE), priority{0} {}
    explicit Command(IdType id, int priority = 0, unsigned char mask = 0, unsigned char param1 = 0,
            unsigned char param2 = 0)
        : id(id), priority{priority}, params{mask, param1, param2} {}
    static NumberType idAsNumber(IdType id) { return as_number(id); }

    IdType id;
    int priority;
    unsigned char params[3];

    Command & operator|=(const Command &other);

    bool operator==(const Command &other) const
    {
        if (id != other.id) {
            return false;
        }
        for (int i = 0; i < 3; ++i) {
            if (params[i] != other.params[i]) {
                return false;
            }
        }
        return true;
    }

    bool operator!=(const Command &other) const { return !(*this == other); }

    bool isCompatible(const Command &other) const;
};

}  // namespace impl_
}  // namespace k8090
}  // namespace core
}  // namespace sprelay

#endif  // SPRELAY_CORE_K8090_UTILS_H_

