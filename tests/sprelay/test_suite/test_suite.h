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

/*!
 * \brief Container type used for storage of tests declaration
 */
using TestContainer = std::map<std::string, std::unique_ptr<QObject>>;


/*!
 * \brief Returns the tests storage container static instance.
 * \return The test storage container.
 *
 * This method can be used to insert tests inside test container for the later execution.
 * \code
 * auto &tests = get_tests();
 * if (!tests.count(name)) {
 *     tests.insert(std::make_pair(name, std::unique_ptr<TestClass>(new TestClass)));
 * }
 * \endcode
 */
inline TestContainer & get_tests()
{
    static TestContainer map;
    return map;
}


/*!
 * \brief Runs all tests from the test storage.
 * \param argc The number of command line arguments.
 * \param argv Command line arguments.
 * \return Logic or of all the returned statuses.
 *
 * Example:
 * \code
 * #include <QCoreApplication>
 * #include "sprelay/test_suite/test_suite.h"
 *
 * int main(int argc, char **argv)
 * {
 *     QCoreApplication app(argc, argv);
 *     return sprelay::test_suite::run_tests(argc, argv);
 * }
 * \endcode
 * \sa ADD_TEST(class_name), sprelay::test_suite::TestInserter
 */
inline int run_tests(int argc, char **argv) {
    int status = 0;
    for (const auto &i : get_tests()) {
        status |= QTest::qExec(i.second.get(), argc, argv);
    }
    return status;
}


/*!
 * \brief Class template for test insertion.
 *
 * Usage:
 * \code
 * #include <QObject>
 * #include <QtTest>
 *
 * #include "sprelay/test_suite/test_suite.h"
 *
 * class TestClass : public QObject
 * {
 *     Q_OBJECT
 *
 * private slots:
 *     void testCase() { QVERIFY(true); }
 * };
 *
 * static sprelay::test_suite::TestInserter<TestClass> test_class{"test id name"};
 * \endcode
 * \sa ADD_TEST(class_name), run_tests()
 */
template <typename TestClass>
struct TestInserter {
    /*!
     * \brief Inserts test with the specified name into test container.
     * \param name The test name.
     * \sa get_tests()
     */
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

/*!
 * \brief Macro for adding tests.
 *
 * The macro defines static variable with internal scope and with the name
 * Usage:
 * \code
 * #include <QObject>
 * #include <QtTest>
 *
 * #include "sprelay/test_suite/test_suite.h"
 *
 * namespace test_class_namespace {
 *
 * class TestClass : public QObject
 * {
 *     Q_OBJECT
 *
 * private slots:
 *     void testCase() { QVERIFY(true); }
 * };
 *
 * ADD_TEST(TestClass);
 *
 * }  // namespace test_class_namespace
 * \endcode
 * \sa sprelay::test_suite::TestInserter, run_tests()
 */
#define ADD_TEST(class_name) static sprelay::test_suite::TestInserter<class_name> test_##class_name{#class_name};

#endif  // SPRELAY_TEST_SUITE_TEST_SUITE_H_
