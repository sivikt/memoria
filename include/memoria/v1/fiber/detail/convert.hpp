
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_FIBERS_DETAIL_CONVERT_H
#define MEMORIA_FIBERS_DETAIL_CONVERT_H

#include <chrono>
#include <memory>

#include <boost/config.hpp>

#include <memoria/v1/fiber/detail/config.hpp>

#ifdef BOOST_HAS_ABI_HEADERS
#  include MEMORIA_BOOST_ABI_PREFIX
#endif

namespace memoria { namespace v1 {
namespace fibers {
namespace detail {

inline
std::chrono::steady_clock::time_point convert(
        std::chrono::steady_clock::time_point const& timeout_time) noexcept {
    return timeout_time;
}

template< typename Clock, typename Duration >
std::chrono::steady_clock::time_point convert(
        std::chrono::time_point< Clock, Duration > const& timeout_time) {
    return std::chrono::steady_clock::now() + ( timeout_time - Clock::now() );
}

}}}}

#ifdef BOOST_HAS_ABI_HEADERS
#  include MEMORIA_BOOST_ABI_SUFFIX
#endif

#endif // MEMORIA_FIBERS_DETAIL_CONVERT_H
