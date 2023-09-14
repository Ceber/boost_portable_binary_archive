#ifndef PORTABLE_BINARY_IARCHIVE_HPP
#define PORTABLE_BINARY_IARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
#pragma once
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// portable_binary_iarchive.hpp

// (C) Copyright 2002-7 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <boost/archive/archive_exception.hpp>
#include <boost/archive/basic_binary_iprimitive.hpp>
#include <boost/archive/detail/common_iarchive.hpp>
#include <boost/archive/detail/register_archive.hpp>
#include <boost/serialization/item_version_type.hpp>
#include <boost/serialization/string.hpp>
#include <istream>
#include <type_traits>

#include "portable_binary_archive.hpp"

namespace boost {
namespace archive {

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// exception to be thrown if integer read from archive doesn't fit
// variable being loaded
class portable_binary_iarchive_exception : public boost::archive::archive_exception {
public:
  enum exception_code { incompatible_integer_size, no_infnan, has_denorm } m_exception_code;
  portable_binary_iarchive_exception(exception_code c = incompatible_integer_size)
      : boost::archive::archive_exception(boost::archive::archive_exception::other_exception), m_exception_code(c) {}
  virtual const char *what() const throw() {
    const char *msg = "programmer error";
    switch (m_exception_code) {
    case incompatible_integer_size:
      msg = "integer cannot be represented !";
      break;
    case no_infnan:
      msg = "floating points issue: no_infnan !";
      break;
    case has_denorm:
      msg = "floating points issue: has_denorm !";
      break;
    default:
      msg = boost::archive::archive_exception::what();
      assert(false);
      break;
    }
    return msg;
  }
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// "Portable" input binary archive.  It addresses integer size and endienness so
// that binary archives can be passed across systems. Note:floating point types
// not addressed here
class portable_binary_iarchive : public boost::archive::basic_binary_iprimitive<portable_binary_iarchive, std::istream::char_type,
                                                                                std::istream::traits_type>,
                                 public boost::archive::detail::common_iarchive<portable_binary_iarchive> {
  typedef boost::archive::basic_binary_iprimitive<portable_binary_iarchive, std::istream::char_type, std::istream::traits_type>
      primitive_base_t;
  typedef boost::archive::detail::common_iarchive<portable_binary_iarchive> archive_base_t;
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
  friend archive_base_t;
  friend primitive_base_t; // since with override load below
  friend class boost::archive::detail::interface_iarchive<portable_binary_iarchive>;
  friend class boost::archive::load_access;

protected:
#endif
  unsigned int m_flags;
  void load_impl(boost::intmax_t &l, char maxsize);

  // default fall through for any types not specified here
  template <class T> void load(T &t) {
    boost::intmax_t l;
    load_impl(l, sizeof(T));
    // use cast to avoid compile time warning
    // t = static_cast< T >(l);
    t = T(l);
  }
  void load(boost::serialization::item_version_type &t) {
    boost::intmax_t l;
    load_impl(l, sizeof(boost::serialization::item_version_type));
    // use cast to avoid compile time warning
    t = boost::serialization::item_version_type(l);
  }
  void load(boost::archive::version_type &t) {
    boost::intmax_t l;
    load_impl(l, sizeof(boost::archive::version_type));
    // use cast to avoid compile time warning
    t = boost::archive::version_type(l);
  }
  void load(boost::archive::class_id_type &t) {
    boost::intmax_t l;
    load_impl(l, sizeof(boost::archive::class_id_type));
    // use cast to avoid compile time warning
    t = boost::archive::class_id_type(static_cast<int>(l));
  }
  void load(std::string &t) { this->primitive_base_t::load(t); }
#ifndef BOOST_NO_STD_WSTRING
  void load(std::wstring &t) { this->primitive_base_t::load(t); }
#endif

  /**
   * \brief Load floating point types.
   *
   * We simply rely on fp_traits to set the bit pattern from the (unsigned)
   * integral type that was stored in the stream. Francois Mauger provided
   * standardized behaviour for special values like inf and NaN, that need to
   * be serialized in his application.
   *
   * \note by Johan Rade (author of the floating point utilities library):
   * Be warned that the math::detail::fp_traits<T>::type::get_bits() function
   * is *not* guaranteed to give you all bits of the floating point number. It
   * will give you all bits if and only if there is an integer type that has
   * the same size as the floating point you are copying from. It will not
   * give you all bits for double if there is no uint64_t. It will not give
   * you all bits for long double if sizeof(long double) > 8 or there is no
   * uint64_t.
   *
   * The member fp_traits<T>::type::coverage will tell you whether all bits
   * are copied. This is a typedef for either math::detail::all_bits or
   * math::detail::not_all_bits.
   *
   * If the function does not copy all bits, then it will copy the most
   * significant bits. So if you serialize and deserialize the way you
   * describe, and fp_traits<T>::type::coverage is math::detail::not_all_bits,
   * then your floating point numbers will be truncated. This will introduce
   * small rounding off errors.
   */
  template <typename T> typename boost::enable_if<boost::is_floating_point<T>>::type load_floating(T &t) {
    typedef typename fp::detail::fp_traits<T>::type traits;

    // if you end here there are three possibilities:
    // 1. you're serializing a long double which is not portable
    // 2. you're serializing a double but have no 64 bit integer
    // 3. your machine is using an unknown floating point format
    // after reading the note above you still might decide to
    // deactivate this static assert and try if it works out.
    typename traits::bits bits;
    BOOST_STATIC_ASSERT(sizeof(bits) == sizeof(T));
    BOOST_STATIC_ASSERT(std::numeric_limits<T>::is_iec559);

    load(bits);
    traits::set_bits(t, bits);

    // if the no_infnan flag is set we must throw here
    if (get_flags() & no_infnan && !fp::isfinite(t))
      throw portable_binary_iarchive_exception(portable_binary_iarchive_exception::no_infnan);

    // if you end here your floating point type does not support
    // denormalized numbers. this might be the case even though
    // your type conforms to IEC 559 (and thus to IEEE 754)
    if (std::numeric_limits<T>::has_denorm == std::denorm_absent && fp::fpclassify(t) == (int)FP_SUBNORMAL) // GCC4
      throw portable_binary_iarchive_exception(portable_binary_iarchive_exception::has_denorm);
  }

  void load(float &t) {
    load_floating(t);
    // this->primitive_base_t::load(t);
    // floats not supported
    // BOOST_STATIC_ASSERT(false);
  }
  void load(double &t) {
    load_floating(t);
    // this->primitive_base_t::load(t);
    // doubles not supported
    // BOOST_STATIC_ASSERT(false);
  }
  void load(char &t) { this->primitive_base_t::load(t); }
  void load(unsigned char &t) { this->primitive_base_t::load(t); }
  typedef boost::archive::detail::common_iarchive<portable_binary_iarchive> detail_common_iarchive;
  template <class T> void load_override(T &t) { this->detail_common_iarchive::load_override(t); }
  void load_override(boost::archive::class_name_type &t);
  // binary files don't include the optional information
  void load_override(boost::archive::class_id_optional_type &) {}

  void init(unsigned int flags);

public:
  portable_binary_iarchive(std::istream &is, unsigned flags = 0)
      : primitive_base_t(*is.rdbuf(), 0 != (flags & boost::archive::no_codecvt)), archive_base_t(flags), m_flags(0) {
    init(flags);
  }

  portable_binary_iarchive(std::basic_streambuf<std::istream::char_type, std::istream::traits_type> &bsb, unsigned int flags)
      : primitive_base_t(bsb, 0 != (flags & boost::archive::no_codecvt)), archive_base_t(flags), m_flags(0) {
    init(flags);
  }
};

} // namespace archive
} // namespace boost

// required by export in boost version > 1.34
#ifdef BOOST_SERIALIZATION_REGISTER_ARCHIVE
BOOST_SERIALIZATION_REGISTER_ARCHIVE(portable_binary_iarchive)
#endif

// required by export in boost <= 1.34
#define BOOST_ARCHIVE_CUSTOM_IARCHIVE_TYPES portable_binary_iarchive

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // PORTABLE_BINARY_IARCHIVE_HPP