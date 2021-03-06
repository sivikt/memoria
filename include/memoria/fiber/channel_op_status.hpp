//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#ifndef MEMORIA_FIBERS_CHANNEL_OP_STATUS_H
#define MEMORIA_FIBERS_CHANNEL_OP_STATUS_H

#include <boost/config.hpp>

#include <memoria/fiber/detail/config.hpp>


namespace memoria {
namespace fibers {

enum class channel_op_status {
    success = 0,
    empty,
    full,
    closed,
    timeout
};

}}


#endif // MEMORIA_FIBERS_CHANNEL_OP_STATUS_H
