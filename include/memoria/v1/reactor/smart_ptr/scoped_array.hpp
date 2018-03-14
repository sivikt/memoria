
//  (C) Copyright Greg Colvin and Beman Dawes 1998, 1999.
//  Copyright (c) 2001, 2002 Peter Dimov
//
//  Distributed under the Boost Software License, Version 1.0. (See
//  accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt)
//
//  See http://www.boost.org/libs/smart_ptr/ for documentation.

#pragma once

#include <boost/config.hpp>
#include <boost/assert.hpp>
#include <boost/checked_delete.hpp>


#include <boost/detail/workaround.hpp>

#include <cstddef>            // for std::ptrdiff_t

namespace memoria {
namespace v1 {
namespace reactor {

// Debug hooks

#if defined(MMA1_SP_ENABLE_DEBUG_HOOKS)

void sp_array_constructor_hook(void * p);
void sp_array_destructor_hook(void * p);

#endif

//  scoped_array extends scoped_ptr to arrays. Deletion of the array pointed to
//  is guaranteed, either on destruction of the scoped_array or via an explicit
//  reset(). Use shared_array or std::vector if your needs are more complex.

template<class T> class scoped_array // noncopyable
{
private:

    T * px;

    scoped_array(scoped_array const &);
    scoped_array & operator=(scoped_array const &);

    typedef scoped_array<T> this_type;

    void operator==( scoped_array const& ) const;
    void operator!=( scoped_array const& ) const;

public:

    typedef T element_type;

    explicit scoped_array( T * p = 0 ) noexcept : px( p )
    {
#if defined(MMA1_SP_ENABLE_DEBUG_HOOKS)
        reactor::sp_array_constructor_hook( px );
#endif
    }

    ~scoped_array() noexcept
    {
#if defined(MMA1_SP_ENABLE_DEBUG_HOOKS)
        reactor::sp_array_destructor_hook( px );
#endif
        boost::checked_array_delete( px );
    }

    void reset(T * p = 0) noexcept
    {
        BOOST_ASSERT( p == 0 || p != px ); // catch self-reset errors
        this_type(p).swap(*this);
    }

    T & operator[](std::ptrdiff_t i) const noexcept
    {
        BOOST_ASSERT( px != 0 );
        BOOST_ASSERT( i >= 0 );
        return px[i];
    }

    T * get() const noexcept
    {
        return px;
    }

// implicit conversion to "bool"
#include <memoria/v1/reactor/smart_ptr/detail/operator_bool.hpp>

    void swap(scoped_array & b) noexcept
    {
        T * tmp = b.px;
        b.px = px;
        px = tmp;
    }
};



template<class T> inline bool operator==( scoped_array<T> const & p, std::nullptr_t ) noexcept
{
    return p.get() == 0;
}

template<class T> inline bool operator==( std::nullptr_t, scoped_array<T> const & p ) noexcept
{
    return p.get() == 0;
}

template<class T> inline bool operator!=( scoped_array<T> const & p, std::nullptr_t ) noexcept
{
    return p.get() != 0;
}

template<class T> inline bool operator!=( std::nullptr_t, scoped_array<T> const & p ) noexcept
{
    return p.get() != 0;
}



template<class T> inline void swap(scoped_array<T> & a, scoped_array<T> & b) noexcept
{
    a.swap(b);
}

}}}
