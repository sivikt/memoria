
//          Copyright Oliver Kowalke 2013.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_FIBERS_DETAIL_TASK_OBJECT_H
#define MEMORIA_FIBERS_DETAIL_TASK_OBJECT_H

#include <exception>
#include <memory>
#include <tuple>
#include <utility>

#include <boost/config.hpp>
#include <memoria/context/detail/config.hpp>
#if defined(BOOST_NO_CXX17_STD_APPLY)
#include <memoria/context/detail/apply.hpp>
#endif
#include <boost/core/pointer_traits.hpp>

#include <memoria/fiber/detail/config.hpp>
#include <memoria/fiber/future/detail/task_base.hpp>

namespace memoria {
namespace fibers {
namespace detail {

template< typename Fn, typename Allocator, typename R, typename ... Args >
class task_object : public task_base< R, Args ... > {
private:
    typedef task_base< R, Args ... >    base_type;
    typedef std::allocator_traits< Allocator >  allocator_traits;

public:
    typedef typename allocator_traits::template rebind_alloc<
        task_object
    >                                           allocator_type;

    task_object( allocator_type const& alloc, Fn const& fn) :
        base_type{},
        fn_{ fn },
        alloc_{ alloc } {
    }

    task_object( allocator_type const& alloc, Fn && fn) :
        base_type{},
        fn_{ std::move( fn) },
        alloc_{ alloc } {
    }

    void run( Args && ... args) override final {
        try {
            this->set_value(
#if defined(BOOST_NO_CXX17_STD_APPLY)
                    memoria::context::detail::apply(
                        fn_, std::make_tuple( std::forward< Args >( args) ... ) )
#else
                    std::apply(
                        fn_, std::make_tuple( std::forward< Args >( args) ... ) )
#endif
                    );
#if defined(MEMORIA_CONTEXT_HAS_CXXABI_H)
        } catch ( abi::__forced_unwind const&) {
            throw;
#endif
        } catch (...) {
            this->set_exception( std::current_exception() );
        }
    }

    typename base_type::ptr_type reset() override final {
        typedef std::allocator_traits< allocator_type >    traity_type;
        typedef ::boost::pointer_traits< typename traity_type::pointer> ptrait_type;

        typename traity_type::pointer ptr{ traity_type::allocate( alloc_, 1) };
        typename ptrait_type::element_type* p = boost::to_address(ptr);
        try {
            traity_type::construct( alloc_, p, alloc_, std::move( fn_) );
        } catch (...) {
            traity_type::deallocate( alloc_, ptr, 1);
            throw;
        }
        return { p };
    }

protected:
    void deallocate_future() noexcept override final {
        destroy_( alloc_, this);
    }

private:
    Fn                  fn_;
    allocator_type      alloc_;

    static void destroy_( allocator_type const& alloc, task_object * p) noexcept {
        allocator_type a{ alloc };
        typedef std::allocator_traits< allocator_type >    traity_type;
        traity_type::destroy( a, p);
        traity_type::deallocate( a, p, 1);
    }
};

template< typename Fn, typename Allocator, typename ... Args >
class task_object< Fn, Allocator, void, Args ... > : public task_base< void, Args ... > {
private:
    typedef task_base< void, Args ... >    base_type;
    typedef std::allocator_traits< Allocator >    allocator_traits;

public:
    typedef typename allocator_traits::template rebind_alloc<
        task_object< Fn, Allocator, void, Args ... >
    >                                             allocator_type;

    task_object( allocator_type const& alloc, Fn const& fn) :
        base_type{},
        fn_{ fn },
        alloc_{ alloc } {
    }

    task_object( allocator_type const& alloc, Fn && fn) :
        base_type{},
        fn_{ std::move( fn) },
        alloc_{ alloc } {
    }

    void run( Args && ... args) override final {
        try {
#if defined(BOOST_NO_CXX17_STD_APPLY)
            memoria::context::detail::apply(
                    fn_, std::make_tuple( std::forward< Args >( args) ... ) );
#else
            std::apply(
                    fn_, std::make_tuple( std::forward< Args >( args) ... ) );
#endif
            this->set_value();
#if defined(MEMORIA_CONTEXT_HAS_CXXABI_H)
        } catch ( abi::__forced_unwind const&) {
            throw;
#endif
        } catch (...) {
            this->set_exception( std::current_exception() );
        }
    }

    typename base_type::ptr_type reset() override final {
        typedef std::allocator_traits< allocator_type >    traity_type;
        typedef ::boost::pointer_traits< typename traity_type::pointer> ptrait_type;

        typename traity_type::pointer ptr{ traity_type::allocate( alloc_, 1) };
        typename ptrait_type::element_type* p = boost::to_address(ptr);
        try {
            traity_type::construct( alloc_, p, alloc_, std::move( fn_) );
        } catch (...) {
            traity_type::deallocate( alloc_, ptr, 1);
            throw;
        }
        return { p };
    }

protected:
    void deallocate_future() noexcept override final {
        destroy_( alloc_, this);
    }

private:
    Fn                  fn_;
    allocator_type      alloc_;

    static void destroy_( allocator_type const& alloc, task_object * p) noexcept {
        allocator_type a{ alloc };
        typedef std::allocator_traits< allocator_type >    traity_type;
        traity_type::destroy( a, p);
        traity_type::deallocate( a, p, 1);
    }
};

}}}

#endif // MEMORIA_FIBERS_DETAIL_TASK_OBJECT_H
