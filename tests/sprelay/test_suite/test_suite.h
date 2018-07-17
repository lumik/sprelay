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
 * \file      test_suite.h
 * \brief     Test suite for automated test addition.

 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-17
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 *
 * \copyright Inspired by
 * https://stackoverflow.com/questions/12194256/qt-how-to-organize-unit-test-with-more-than-one-class
 */


#ifndef SPRELAY_TEST_SUITE_TEST_SUITE_H_
#define SPRELAY_TEST_SUITE_TEST_SUITE_H_

#include <QtTest>

#include <memory>
#include <map>
#include <string>

namespace sprelay {
namespace test_suite{

// TODO(lumik): make documentation

using TestContainer = std::map<std::string, std::unique_ptr<QObject>>;

inline TestContainer & get_tests()
{
    static TestContainer map;
    return map;
}


inline int run_tests(int argc, char **argv) {
    int status = 0;
    for (const auto &i : get_tests()) {
        status |= QTest::qExec(i.second.get(), argc, argv);
    }
    return status;
}


template <typename TestClass>
class TestInserter {
public:  // NOLINT(whitespace/indent)
    explicit TestInserter(const char *name)
    {
        auto &tests = get_tests();
        if (!tests.count(name)) {
            tests.insert(std::make_pair(name, std::unique_ptr<TestClass>(new TestClass)));
        }
    }
};

}  // namespace test_suite
}  // namespace sprelay

#define ADD_TEST(class_name) static sprelay::test_suite::TestInserter<class_name> test_##class_name{#class_name};

#endif  // SPRELAY_TEST_SUITE_TEST_SUITE_H_

