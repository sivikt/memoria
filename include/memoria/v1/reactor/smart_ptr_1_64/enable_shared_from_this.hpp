
//
//  enable_shared_from_this.hpp
//
//  Copyright 2002, 2009 Peter Dimov
//  Copyright 2017 Victor Smirnov
//
//  Distributed under the Boost Software License, Version 1.0.
//  See accompanying file LICENSE_1_0.txt or copy at
//  http://www.boost.org/LICENSE_1_0.txt
//
//  http://www.boost.org/libs/smart_ptr/enable_shared_from_this.html
//

#pragma once

#include <memoria/v1/reactor/smart_ptr/weak_ptr.hpp>
#include <memoria/v1/reactor/smart_ptr/shared_ptr.hpp>
#include <boost/assert.hpp>
#include <boost/config.hpp>

namespace memoria {
namespace v1 {
namespace reactor_1_64 {


template<class T> class enable_shared_from_this
{
protected:

    enable_shared_from_this() noexcept
    {
    }

    enable_shared_from_this(enable_shared_from_this const &) noexcept
    {
    }

    enable_shared_from_this & operator=(enable_shared_from_this const &) noexcept
    {
        return *this;
    }

    ~enable_shared_from_this() noexcept // ~weak_ptr<T> newer throws, so this call also must not throw
    {
    }

public:

    shared_ptr<T> shared_from_this()
    {
        shared_ptr<T> p( weak_this_ );
        BOOST_ASSERT( p.get() == this );
        return p;
    }

    shared_ptr<T const> shared_from_this() const
    {
        shared_ptr<T const> p( weak_this_ );
        BOOST_ASSERT( p.get() == this );
        return p;
    }

    weak_ptr<T> weak_from_this() noexcept
    {
        return weak_this_;
    }

    weak_ptr<T const> weak_from_this() const noexcept
    {
        return weak_this_;
    }


public:


    // Note: invoked automatically by shared_ptr; do not call
    template<class X, class Y> void _internal_accept_owner( shared_ptr<X> const * ppx, Y * py ) const
    {
        if( weak_this_.expired() )
        {
            weak_this_ = shared_ptr<T>( *ppx, py );
        }
    }

private:

    mutable weak_ptr<T> weak_this_;
};

}}}
