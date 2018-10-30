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
 * \file      sprelay_global.h
 * \ingroup   group_sprelay_public
 * \brief     Sprelay global staff.
 *
 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-03
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef BIOMOLECULES_SPRELAY_SPRELAY_GLOBAL_H_
#define BIOMOLECULES_SPRELAY_SPRELAY_GLOBAL_H_

#include <QtCore/QtGlobal>

#if defined(SPRELAY_STANDALONE)
    #define SPRELAY_EXPORT
#else  // if defined(SPRELAY_STANDALONE)
    #if defined(SPRELAY_MAIN_LIBRARY)
        #define SPRELAY_EXPORT Q_DECL_EXPORT
    #else  // if defined(SPRELAY_MAIN_LIBRARY)
        #define SPRELAY_EXPORT Q_DECL_IMPORT
    #endif  // if defined(SPRELAY_MAIN_LIBRARY)
#endif  // if defined(SPRELAY_STANDALONE)


#if defined(SPRELAY_LIBRARY)
    #define SPRELAY_LIBRARY_EXPORT Q_DECL_EXPORT
#else  // if defined(SPRELAY_BUILD_LIBRARY)
    #define SPRELAY_LIBRARY_EXPORT Q_DECL_IMPORT
#endif  // if defined(SPRELAY_BUILD_LIBRARY)


// Documentation
/*!
 * \addtogroup group_sprelay_public
 * @{
 */

/*!
 * \def SPRELAY_EXPORT
 * \brief This macro is used to import symbols from the main `sprelay` shared library.
 *
 * Its behavior can be changed if you define either `#SPRELAY_STANDALONE` or `#SPRELAY_MAIN_LIBRARY` macro in `sprelay`
 * source code.
 *
 * Example usage:
 * \code
 * SPRELAY_EXPORT void foo();
 * class SPRELAY_EXPORT MyClass {};
 * \endcode
 * \sa SPRELAY_LIBRARY_EXPORT
 */

/*!
 * \def SPRELAY_LIBRARY_EXPORT
 * \brief This macro is used to import symbols from internal `sprelay` shared libraries.
 *
 * Its behavior can be changed if you define `#SPRELAY_LIBRARY` macro in `sprelay` source code.
 *
 * Example usage:
 * \code
 * SPRELAY_LIBRARY_EXPORT void foo();
 * class SPRELAY_LIBRARY_EXPORT MyClass {};
 * \endcode
 * \sa SPRELAY_EXPORT
 */

/*! @}*/  // group_sprelay_public

#if defined(DOXYGEN)
    #define SPRELAY_STANDALONE
#endif
/*!
 * \def SPRELAY_STANDALONE
 * \brief Define this macro if you want to compile `sprelay` as a standalone application.
 *
 * You can define it for example by passing `-DSPRELAY_STANDALONE` flag to compiler.
 *
 * This macro modifies the behavior of `#SPRELAY_EXPORT` macro to not act as shared library compilation vs client code
 * switch.
 *
 * \warning This macro shouldn't be defined inside client code.
 *
 * \sa SPRELAY_EXPORT and SPRELAY_LIBRARY_EXPORT macros for more details about this macro effect.
 */

#if defined(DOXYGEN)
    #define SPRELAY_MAIN_LIBRARY
#endif
/*!
 * \def SPRELAY_MAIN_LIBRARY
 * \brief Define this macro if you want to change the behavior of `#SPRELAY_EXPORT` macro to compile `sprelay` as a
 * shared library.
 *
 * If you want the standalone application instead, you have to define `#SPRELAY_STANDALONE` macro. In client code, when
 * you only use the shared library, you must not define either of these two macros.
 *
 * You can define the `#SPRELAY_MAIN_LIBRARY` macro for example by passing `-DSPRELAY_MAIN_LIBRARY` flag to compiler.
 *
 * Every declaration of symbol, which should be exported from the main shared library (i.e. code which is compiled as a
 * shared library or into standalone application according to the compilation settings) should contain the
 * `#SPRELAY_EXPORT` macro. The `#SPRELAY_LIBRARY_EXPORT` macro should be used instead for code which should always be
 * compiled as a shared library (despite the fact that the main part of the application is compiled as shared library
 * or a standalone application).
 *
 * Typically, clients will include only the public header files of shared libraries. These libraries might be installed
 * in a different location, when deployed. Therefore, it is important to exclude other internal header files that were
 * used when building the shared library. This can be avoided by making use of the Pointer to implementation idiom
 * described in various C++ programming books.
 *
 * \warning This macro shouldn't be defined in client code.
 *
 * \sa `#SPRELAY_EXPORT` and `#SPRELAY_LIBRARY_EXPORT` macros for more details about this macro effect.
 */

#if defined(DOXYGEN)
    #define SPRELAY_LIBRARY
#endif
/*!
 * \def SPRELAY_LIBRARY
 * \brief Define this macro if you want to change the behavior of `#SPRELAY_LIBRARY_EXPORT` macro to compile the parts
 * of code marked by the `#SPRELAY_LIBRARY_EXPORT` macro as an internal shared library.
 *
 * You can define it for example by passing `-DSPRELAY_LIBRARY_EXPORT` flag to compiler. In client code, when you only
 * use the shared library, you must not define this macro.
 *
 * Every declaration of symbol which should be exported from an internal shared library (i.e. code which is always
 * compiled as a shared library despite the application settings) should also contain `#SPRELAY_LIBRARY_EXPORT` macro.
 * This macro should not be used for parts of code which can be compiled as shared library or into executable binary
 * depending on compilation settings. The #SPRELAY_EXPORT macro should be use there instead.
 *
 * Typically, clients will include only the public header files of shared libraries. These libraries might be installed
 * in a different location, when deployed. Therefore, it is important to exclude other internal header files that were
 * used when building the shared library. This can be avoided by making use of the Pointer to implementation idiom
 * described in various C++ programming books.
 *
 * \warning This macro shouldn't be defined in client code.
 *
 * \sa `#SPRELAY_EXPORT` and `#SPRELAY_LIBRARY_EXPORT` macros for more details about this macro effect.
 */

#endif  // BIOMOLECULES_SPRELAY_SPRELAY_GLOBAL_H_
