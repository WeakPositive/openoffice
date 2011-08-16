/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef INCLUDED_BASEBMP_METAFUNCTIONS_HXX
#define INCLUDED_BASEBMP_METAFUNCTIONS_HXX

#include <boost/mpl/integral_c.hpp>
#include <vigra/metaprogramming.hxx>
#include <vigra/numerictraits.hxx>

namespace basebmp
{

// TODO(Q3): move to generic place (o3tl?)    

/** template meta function: add const qualifier to 2nd type, if given
    1st type has it
*/
template<typename A, typename B> struct clone_const 
{
    typedef B type;
};
template<typename A, typename B> struct clone_const<const A,B>
{
    typedef const B type;
};

/** template meta function: add const qualifier to plain type (if not
    already there)
 */
template <typename T> struct add_const 
{
    typedef const T type;
};
template <typename T> struct add_const<const T>
{
    typedef const T type;
};

/// template meta function: remove const qualifier from plain type
template <typename T> struct remove_const 
{
    typedef T type;
};
template <typename T> struct remove_const<const T>
{
    typedef T type;
};

//--------------------------------------------------------------

/// Base class for an adaptable ternary functor
template< typename A1, typename A2, typename A3, typename R > struct TernaryFunctorBase
{
    typedef A1 first_argument_type;
    typedef A2 second_argument_type;
    typedef A3 third_argument_type;
    typedef R  result_type;
};

//--------------------------------------------------------------

/** template meta function: ensure that given integer type is unsigned

    If given integer type is already unsigned, return as-is -
    otherwise, convert to unsigned type of same or greater range.
 */
template< typename T > struct make_unsigned;

#define BASEBMP_MAKE_UNSIGNED(T,U)        \
    template<> struct make_unsigned<T> { \
        typedef U type; \
    };

BASEBMP_MAKE_UNSIGNED(signed char,unsigned char)
BASEBMP_MAKE_UNSIGNED(unsigned char,unsigned char)
BASEBMP_MAKE_UNSIGNED(short,unsigned short)
BASEBMP_MAKE_UNSIGNED(unsigned short,unsigned short)
BASEBMP_MAKE_UNSIGNED(int,unsigned int)
BASEBMP_MAKE_UNSIGNED(unsigned int,unsigned int)
BASEBMP_MAKE_UNSIGNED(long,unsigned long)
BASEBMP_MAKE_UNSIGNED(unsigned long,unsigned long)

#undef BASEBMP_MAKE_UNSIGNED

/// cast integer to unsigned type of similar size
template< typename T > inline typename make_unsigned<T>::type unsigned_cast( T value )
{
    return static_cast< typename make_unsigned<T>::type >(value);
}

//--------------------------------------------------------------

/// returns true, if given number is strictly less than 0
template< typename T > inline bool is_negative( T x )
{
    return x < 0;
}

/// Overload for ints (branch-free)
inline bool is_negative( int x )
{
    // force logic shift (result for signed shift right is undefined)
    return static_cast<unsigned int>(x) >> (sizeof(int)*8-1);
}

//--------------------------------------------------------------

/// Results in VigraTrueType, if T is of integer type and scalar
template< typename T, typename trueCase, typename falseCase >
struct ifScalarIntegral
{
    typedef  
    typename vigra::If< 
        typename vigra::NumericTraits< T >::isIntegral,
        typename vigra::If< 
            typename vigra::NumericTraits< T >::isScalar,
            trueCase,
            falseCase >::type,
        falseCase >::type type;
};

/// Results in VigraTrueType, if T is of non-integer type and scalar
template< typename T, typename trueCase, typename falseCase >
struct ifScalarNonIntegral
{
    typedef  
    typename vigra::If< 
        typename vigra::NumericTraits< T >::isIntegral,
        falseCase,
        typename vigra::If< 
            typename vigra::NumericTraits< T >::isScalar,
            trueCase,
            falseCase >::type >::type type;
};

/// Results in VigraTrueType, if both T1 and T2 are of integer type and scalar
template< typename T1, typename T2, typename trueCase, typename falseCase >
struct ifBothScalarIntegral
{
    typedef  
    typename ifScalarIntegral< 
        T1, 
        typename ifScalarIntegral< 
            T2, 
            trueCase,
            falseCase >::type,
        falseCase >::type type;
};

//--------------------------------------------------------------

/// Count number of trailing zeros
template< unsigned int val > struct numberOfTrailingZeros
{
    enum { next = val >> 1 };
    enum { value = vigra::IfBool< (val & 1) == 0,
                                  numberOfTrailingZeros<next>,
                                  boost::mpl::integral_c< int,-1 > > ::type::value + 1 };
};

template<> struct numberOfTrailingZeros<0>
{
    enum { value = 0 };
};

//--------------------------------------------------------------

/// Count number of one bits 
template< unsigned int val > struct bitcount
{
    enum { next = val >> 1 };
    enum { value = bitcount<next>::value + (val & 1) };
};

template<> struct bitcount<0>
{
    enum { value = 0 };
};

//--------------------------------------------------------------

/// Shift left for positive shift value, and right otherwise 
template< typename T > inline T shiftLeft( T v, int shift )
{
    return shift > 0 ? v << shift : v >> (-shift);
}

/// Shift right for positive shift value, and left otherwise 
template< typename T > inline T shiftRight( T v, int shift )
{
    return shift > 0 ? v >> shift : v << (-shift);
}


} // namespace basebmp

#endif /* INCLUDED_BASEBMP_METAFUNCTIONS_HXX */
