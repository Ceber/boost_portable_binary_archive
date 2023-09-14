#ifndef PORTABLE_BINARY_ARCHIVE_HPP
#define PORTABLE_BINARY_ARCHIVE_HPP

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
#pragma once
#endif

#include <boost/config.hpp>
#include <boost/cstdint.hpp>
#include <boost/static_assert.hpp>

#include <climits>
#if CHAR_BIT != 8
#error This code assumes an eight-bit byte.
#endif

#include <boost/archive/basic_archive.hpp>
#include <boost/math_fwd.hpp>
#include <boost/predef/other/endian.h>
#include <boost/utility/enable_if.hpp>

// endian and fpclassify
#if BOOST_VERSION < 103600
#include <boost/integer/endian.hpp>
#include <boost/math/fpclassify.hpp>
#elif BOOST_VERSION < 104800
#include <boost/spirit/home/support/detail/integer/endian.hpp>
#include <boost/spirit/home/support/detail/math/fpclassify.hpp>
#elif BOOST_VERSION >= 106900
#define BOOST_MATH_DISABLE_STD_FPCLASSIFY
#include <boost/endian/conversion.hpp>
#include <boost/math/special_functions/fpclassify.hpp>
#else
#include <boost/spirit/home/support/detail/endian/endian.hpp>
#include <boost/spirit/home/support/detail/math/fpclassify.hpp>
#endif

// namespace alias
#if BOOST_VERSION < 103800
namespace fp = boost::math;
#elif BOOST_VERSION >= 106900
namespace fp = boost::math;
#else
namespace fp = boost::spirit::math;
#endif

// flag for fp serialization
const unsigned no_infnan = 64;

enum portable_binary_archive_flags { endian_big = 0x4000, endian_little = 0x8000 };

// #if ( endian_big <= boost::archive::flags_last )
// #error archive flags conflict
// #endif

inline void reverse_bytes(char size, char *address) {
  char *first = address;
  char *last = first + size - 1;
  for (; first < last; ++first, --last) {
    char x = *last;
    *last = *first;
    *first = x;
  }
}

#endif // PORTABLE_BINARY_ARCHIVE_HPP