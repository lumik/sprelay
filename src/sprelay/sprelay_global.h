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

 * \author    Jakub Klener <lumiksro@centrum.cz>
 * \date      2018-07-03
 * \copyright Copyright (C) 2018 Jakub Klener. All rights reserved.
 *
 * \copyright This project is released under the 3-Clause BSD License. You should have received a copy of the 3-Clause
 *            BSD License along with this program. If not, see https://opensource.org/licenses/.
 */


#ifndef SPRELAY_SPRELAY_GLOBAL_H_
#define SPRELAY_SPRELAY_GLOBAL_H_

#include <QtCore/QtGlobal>

/*!
 * \addtogroup group_sprelay_public
 * @{
 */

/*!
 * \def SPRELAY_EXPORT
 * \brief This macro is used to export symbols if the whole `sprelay` is compiled as shared library.
 *
 * Every declaration of symbol, which should be exported from the shared library should also contain this macro. You
 * can also compile only part of the application as shared library. There should be used #SPRELAY_LIBRARY_EXPORT macro
 * instead for code which should be compiled as shared library each time (despite the fact that the whole application
 * is compiled as shared library or a standalone application).
 *
 * Typically, clients will include only the public header files of shared libraries. These libraries might be installed
 * in a different location, when deployed. Therefore, it is important to exclude other internal header files that were
 * used when building the shared library. This can be avoided by making use of the Pointer to implementation idiom
 * described in various C++ programming books.
 *
 * Example usage:
 *
 * \code
 * SPRELAY_EXPORT void foo();
 * class SPRELAY_EXPORT MyClass {};
 * \endcode
 * \sa SPRELAY_LIBRARY_EXPORT
 */
#if defined(SPRELAY_STANDALONE)
    #define SPRELAY_EXPORT
#else  // if defined(SPRELAY_STANDALONE)
    #if defined(SPRELAY_GLOBAL_LIBRARY)
        /*! This macro switches between library compilation or its importing. */
        #define SPRELAY_EXPORT Q_DECL_EXPORT
    #else  // if defined(SPRELAY_BUILD_LIBRARY)
        #define SPRELAY_EXPORT Q_DECL_IMPORT
    #endif  // if defined(SPRELAY_BUILD_LIBRARY)
#endif  // if defined(SPRELAY_STANDALONE)


/*!
 * \def SPRELAY_LIBRARY_EXPORT
 * \brief This macro is used to export symbols from shared library.
 *
 * Every declaration of symbol which should be exported from the shared library should also contain this macro. This
 * macro should not be used for parts of code which can be compiled as shared library or into executable binary
 * depending on compilation settings. The #SPRELAY_EXPORT macro should be use there instead.
 *
 * Typically, clients will include only the public header files of shared libraries. These libraries might be installed
 * in a different location, when deployed. Therefore, it is important to exclude other internal header files that were
 * used when building the shared library. This can be avoided by making use of the Pointer to implementation idiom
 * described in various C++ programming books.
 *
 * Example usage:
 *
 * \code
 * SPRELAY_LIBRARY_EXPORT void foo();
 * class SPRELAY_LIBRARY_EXPORT MyClass {};
 * \endcode
 * \sa SPRELAY_EXPORT
 */
#if defined(SPRELAY_LIBRARY)
    /*! This macro switches between library compilation or its importing. */
    #define SPRELAY_LIBRARY_EXPORT Q_DECL_EXPORT
#else  // if defined(SPRELAY_BUILD_LIBRARY)
    #define SPRELAY_LIBRARY_EXPORT Q_DECL_IMPORT
#endif  // if defined(SPRELAY_BUILD_LIBRARY)

/*! @}*/  // group_sprelay_public

#endif  // SPRELAY_SPRELAY_GLOBAL_H_

