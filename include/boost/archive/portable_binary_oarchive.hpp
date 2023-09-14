#ifndef PORTABLE_BINARY_OARCHIVE_HPP
#define PORTABLE_BINARY_OARCHIVE_HPP

// MS compatible compilers support #pragma once
#if defined(_MSC_VER)
#pragma once
#endif

#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4244)
#endif

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// portable_binary_oarchive.hpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.
#include <boost/archive/basic_binary_oprimitive.hpp>
#include <boost/archive/detail/common_oarchive.hpp>
#include <boost/archive/detail/register_archive.hpp>
#include <boost/serialization/string.hpp>
#include <ostream>
#include <type_traits>

#include "portable_binary_archive.hpp"

namespace boost {
namespace archive {

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// exception to be thrown if integer read from archive doesn't fit
// variable being loaded
class portable_binary_oarchive_exception : public boost::archive::archive_exception {
public:
  enum exception_code { invalid_flags, no_infnan, floating_point_issue } m_exception_code;
  portable_binary_oarchive_exception(exception_code c = invalid_flags)
      : boost::archive::archive_exception(boost::archive::archive_exception::other_exception), m_exception_code(c) {}
  virtual const char *what() const throw() {
    const char *msg = "programmer error";
    switch (m_exception_code) {
    case invalid_flags:
      msg = "cannot be both big and little endian !";
      break;
    case no_infnan:
      msg = "floating points issue: no_infnan !";
      break;
    case floating_point_issue:
      msg = "floating points issue: not able to get bits !";
      break;
    default:
      msg = boost::archive::archive_exception::what();
      break;
    }
    return msg;
  }
};

/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// "Portable" output binary archive.  This is a variation of the native binary
// archive. it addresses integer size and endienness so that binary archives can
// be passed across systems. Note:floating point types not addressed here

class portable_binary_oarchive : public boost::archive::basic_binary_oprimitive<portable_binary_oarchive, std::ostream::char_type,
                                                                                std::ostream::traits_type>,
                                 public boost::archive::detail::common_oarchive<portable_binary_oarchive> {
  typedef boost::archive::basic_binary_oprimitive<portable_binary_oarchive, std::ostream::char_type, std::ostream::traits_type>
      primitive_base_t;
  typedef boost::archive::detail::common_oarchive<portable_binary_oarchive> archive_base_t;
#ifndef BOOST_NO_MEMBER_TEMPLATE_FRIENDS
public:
#else
  friend archive_base_t;
  friend primitive_base_t; // since with override save below
  friend class boost::archive::detail::interface_oarchive<portable_binary_oarchive>;
  friend class boost::archive::save_access;

protected:
#endif
  unsigned int m_flags;
  void save_impl(const boost::intmax_t l, const char maxsize);
  // add base class to the places considered when matching
  // save function to a specific set of arguments.  Note, this didn't
  // work on my MSVC 7.0 system so we use the sure-fire method below
  // using archive_base_t::save;

  // default fall through for any types not specified here
  template <class T> void save(const T &t) { save_impl(t, sizeof(T)); }
  void save(const std::string &t) { this->primitive_base_t::save(t); }
#ifndef BOOST_NO_STD_WSTRING
  void save(const std::wstring &t) { this->primitive_base_t::save(t); }
#endif

  /**
   * \brief Save floating point types.
   *
   * We simply rely on fp_traits to extract the bit pattern into an (unsigned)
   * integral type and store that into the stream. Francois Mauger provided
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
  template <typename T> typename boost::enable_if<boost::is_floating_point<T>>::type save_floating(const T &t) {
    typedef typename fp::detail::fp_traits<T>::type traits;

    // if the no_infnan flag is set we must throw here
    if (get_flags() & no_infnan && !fp::isfinite(t))
      throw portable_binary_oarchive_exception(portable_binary_oarchive_exception::no_infnan);

    // if you end here there are three possibilities:
    // 1. you're serializing a long double which is not portable
    // 2. you're serializing a double but have no 64 bit integer
    // 3. your machine is using an unknown floating point format
    // after reading the note above you still might decide to
    // deactivate this static assert and try if it works out.
    typename traits::bits bits;
    BOOST_STATIC_ASSERT(sizeof(bits) == sizeof(T));
    BOOST_STATIC_ASSERT(std::numeric_limits<T>::is_iec559);

    // examine value closely
    switch (fp::fpclassify(t)) {
      // case FP_ZERO: bits = 0; break;
#if BOOST_VERSION >= 106900
    case FP_NAN:
      bits = traits::exponent | traits::significand;
      break;
#else
    case FP_NAN:
      bits = traits::exponent | traits::mantissa;
      break;
#endif
    case FP_INFINITE:
      bits = traits::exponent | (t < 0) * traits::sign;
      break;
    case FP_SUBNORMAL:
      assert(std::numeric_limits<T>::has_denorm); // pass
    case FP_ZERO:                                 // note that floats can be ±0.0
    case FP_NORMAL:
      traits::get_bits(t, bits);
      break;
    default:
      throw portable_binary_oarchive_exception(portable_binary_oarchive_exception::floating_point_issue);
    }

    save(bits);
  }

  void save(const float &t) {
    save_floating(t);
    // this->primitive_base_t::save(t);
    // floats not supported
    // BOOST_STATIC_ASSERT(false);
  }
  void save(const double &t) {
    save_floating(t);
    // this->primitive_base_t::save(t);
    // doubles not supported
    // BOOST_STATIC_ASSERT(false);
  }
  void save(const char &t) { this->primitive_base_t::save(t); }
  void save(const unsigned char &t) { this->primitive_base_t::save(t); }

  // default processing - kick back to base class.  Note the
  // extra stuff to get it passed borland compilers
  typedef boost::archive::detail::common_oarchive<portable_binary_oarchive> detail_common_oarchive;
  template <class T> void save_override(T &t) { this->detail_common_oarchive::save_override(t); }
  // explicitly convert to char * to avoid compile ambiguities
  void save_override(const boost::archive::class_name_type &t) {
    const std::string s(t);
    *this << s;
  }
  // binary files don't include the optional information
  void save_override(const boost::archive::class_id_optional_type & /* t */
  ) {}

  void init(unsigned int flags);

public:
  portable_binary_oarchive(std::ostream &os, unsigned flags = 0)
      : primitive_base_t(*os.rdbuf(), 0 != (flags & boost::archive::no_codecvt)), archive_base_t(flags),
        m_flags(flags & (endian_big | endian_little)) {
    init(flags);
  }

  portable_binary_oarchive(std::basic_streambuf<std::ostream::char_type, std::ostream::traits_type> &bsb, unsigned int flags)
      : primitive_base_t(bsb, 0 != (flags & boost::archive::no_codecvt)), archive_base_t(flags), m_flags(0) {
    init(flags);
  }
};

} // namespace archive
} // namespace boost

// required by export in boost version > 1.34
#ifdef BOOST_SERIALIZATION_REGISTER_ARCHIVE
BOOST_SERIALIZATION_REGISTER_ARCHIVE(portable_binary_oarchive)
#endif

// required by export in boost <= 1.34
#define BOOST_ARCHIVE_CUSTOM_OARCHIVE_TYPES portable_binary_oarchive

#if defined(_MSC_VER)
#pragma warning(pop)
#endif

#endif // PORTABLE_BINARY_OARCHIVE_HPP