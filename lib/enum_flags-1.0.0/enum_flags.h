/***************************************************************************
**                                                                        **
**  Overloades for bitwise operators for scoped enumerations.             **
**  Copyright (C) 2017 Jakub Klener                                       **
**                                                                        **
**  This file is part of enum_flags project.                              **
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
    \file enum_flags.h
    \brief Templates for bitwise operators are defined in this file.

    If you want bitwise operators to work for your enumeration, you have to overload enableBitmaskOperators() function
    to return true. The library utilizes _SFINAE_ concept, specifically the fact, that interpretation of bitwise
    operators fails when enableBitmasOperators() function returns `false` for used datatype.

    \code
    enum struct TestFlags : unsigned char
    {
        One   = 1 << 0,
        Two   = 1 << 1,
    };

    constexpr bool enableBitmaskOperators(TestFlags) { return true; }

    int main(int argc, char **argv) {
        TestFlags a, b, c;
        a = TestFlags::One;
        b = TestFlags::Two;
        c = a | b;
    }
    \endcode

    \sa enableBitmaskOperators()
*/


#ifndef ENUM_FLAGS_H_
#define ENUM_FLAGS_H_

#include<type_traits>


/*!
    \brief This function template has to be overloaded in order to allow bitwise operators usage for user defined
    enumeration.
    \param E dummy parameter for template function resolution.
    \return true if bitwise operators should be enabled for this enum type.

    If you want bitwise operators to work for your enumeration, you have to overload this function to return true.

    \code
    enum struct TestFlags : unsigned char
    {
        One   = 1 << 0,
        Two   = 1 << 1,
    };

    constexpr bool enableBitmaskOperators(TestFlags) { return true; }

    int main(int argc, char **argv) {
        TestFlags a, b, c;
        a = TestFlags::One;
        b = TestFlags::Two;
        c = a | b;
    }
    \endcode
 */
template<typename E>
constexpr bool enableBitmaskOperators(E) { return false; }


/*!
    \brief Overloaded bitwise operator|().
    \param a left operand.
    \param b right operand.
    \return resulting value of type `E`.
    \sa operator&(), operator^(), operator~(), operator|=().
 */
template<typename E>
inline constexpr typename std::enable_if<enableBitmaskOperators(E()), E>::type
    operator|(E a, E b)
{
    typedef typename std::underlying_type<E>::type underlying;
    return static_cast<E>(static_cast<underlying>(a) | static_cast<underlying>(b));
}


/*!
    \brief Overloaded bitwise operator&().
    \param a left operand.
    \param b right operand.
    \return resulting value of type `E`.
    \sa operator|(), operator^(), operator~(), operator&=().
 */
template<typename E>
inline constexpr typename std::enable_if<enableBitmaskOperators(E()), E>::type
    operator&(E a, E b)
{
    typedef typename std::underlying_type<E>::type underlying;
    return static_cast<E>(static_cast<underlying>(a) & static_cast<underlying>(b));
}


/*!
    \brief Overloaded bitwise operator^().
    \param a left operand.
    \param b right operand.
    \return resulting value of type `E`.
    \sa operator|(), operator&(), operator~(), operator^=.
 */
template<typename E>
inline constexpr typename std::enable_if<enableBitmaskOperators(E()), E>::type
    operator^(E a, E b)
{
    typedef typename std::underlying_type<E>::type underlying;
    return static_cast<E>(static_cast<underlying>(a) ^ static_cast<underlying>(b));
}


/*!
    \brief Overloaded bitwise operator~().
    \param a operand.
    \return resulting value of type `E`.
    \sa operator|(), operator&(), operator^().
 */
template<typename E>
inline typename std::enable_if<enableBitmaskOperators(E()), E>::type
    operator~(E a)
{
    typedef typename std::underlying_type<E>::type underlying;
    return static_cast<E>(~static_cast<underlying>(a));
}


/*!
    \brief Overloaded bitwise operator|=().
    \param a left operand.
    \param b right operand.
    \return resulting value of type `E`.
    \sa operator&=(), operator^=, operator|().
 */
template<typename E>
inline typename std::enable_if<enableBitmaskOperators(E()), E&>::type
    operator|=(E &a, E b)
{
    typedef typename std::underlying_type<E>::type underlying;
    a = static_cast<E>(static_cast<underlying>(a) | static_cast<underlying>(b));
    return a;
}


/*!
    \brief Overloaded bitwise operator&=().
    \param a left operand.
    \param b right operand.
    \return resulting value of type `E`.
    \sa operator|=(), operator^=, operator&().
 */
template<typename E>
inline typename std::enable_if<enableBitmaskOperators(E()), E&>::type
    operator&=(E &a, E b)
{
    typedef typename std::underlying_type<E>::type underlying;
    a = static_cast<E>(static_cast<underlying>(a) & static_cast<underlying>(b));
    return a;
}


/*!
    \brief Overloaded bitwise operator^=().
    \param a left operand.
    \param b right operand.
    \return resulting value of type `E`.
    \sa operator|=(), operator&=, operator^().
*/
template<typename E>
typename std::enable_if<enableBitmaskOperators(E()), E&>::type
    operator^=(E &a, E b)
{
    typedef typename std::underlying_type<E>::type underlying;
    a = static_cast<E>(static_cast<underlying>(a) ^ static_cast<underlying>(b));
    return a;
}

#endif  // ENUM_FLAGS_H_
