/////////1/////////2/////////3/////////4/////////5/////////6/////////7/////////8
// polymorphic_portable_binary_oarchive.cpp

// (C) Copyright 2002 Robert Ramey - http://www.rrsd.com .
// Use, modification and distribution is subject to the Boost Software
// License, Version 1.0. (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

//  See http://www.boost.org for updates, documentation, and revision history.

#include <ostream>

#define BOOST_ARCHIVE_SOURCE
#include "boost/archive/polymorphic_portable_binary_oarchive.hpp"

// explicitly instantiate for this type of text stream
#include <boost/archive/impl/archive_serializer_map.ipp>
#include <boost/archive/impl/basic_binary_oarchive.ipp>
#include <boost/archive/impl/basic_binary_oprimitive.ipp>

namespace boost {
namespace archive {

// explicitly instantiate for this type of binary stream
template class detail::archive_serializer_map<polymorphic_portable_binary_oarchive>;

} // namespace archive
} // namespace boost