
//          Copyright Oliver Kowalke 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#include "memoria/fiber/numa/pin_thread.hpp"

extern "C" {
#include <pthread.h>
#include <sched.h>
}

#include <system_error>


namespace memoria {
namespace fibers {
namespace numa {

MEMORIA_FIBERS_DECL
void pin_thread( std::uint32_t cpuid) {
    pin_thread( cpuid, ::pthread_self() );
}

MEMORIA_FIBERS_DECL
void pin_thread( std::uint32_t cpuid, std::thread::native_handle_type h) {
    cpu_set_t set;
    CPU_ZERO( & set);
    CPU_SET( cpuid, & set);
    int err = 0;
    if ( BOOST_UNLIKELY( 0 != ( err = ::pthread_setaffinity_np( h, sizeof( set), & set) ) ) ) {
        throw std::system_error(
                std::error_code( err, std::system_category() ),
                "pthread_setaffinity_np() failed");
    }
}

}}}
