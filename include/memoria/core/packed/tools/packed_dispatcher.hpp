
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_CORE_PACKED_DISPATCHER_HPP_
#define MEMORIA_CORE_PACKED_DISPATCHER_HPP_

#include <memoria/core/types/types.hpp>
#include <memoria/core/types/type2type.hpp>


#include <memoria/core/packed/tools/packed_allocator.hpp>


namespace memoria {

template <Int StartIdx, typename... List> class PackedDispatcher;

template <Int StartIdx, typename List> struct PackedDispatcherTool;

template <Int StartIdx, typename... List>
struct PackedDispatcherTool<StartIdx, TypeList<List...>> {
    typedef PackedDispatcher<StartIdx, List...> Type;
};



template <typename Struct, Int Index>
struct StreamDescr {
    typedef Struct Type;
};



template <Int StartIdx, typename Head, typename... Tail, Int Index>
class PackedDispatcher<StartIdx, StreamDescr<Head, Index>, Tail...> {

public:
    template <typename Fn, typename... Args>
    static void dispatch(Int idx, PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if (idx == Index)
        {
            Head* head = nullptr;
            if (!alloc->is_empty(Index + StartIdx))
            {
                head = alloc->template get<Head>(Index + StartIdx);
            }

            fn.template stream<Index>(head, args...);
        }
        else {
            PackedDispatcher<StartIdx, Tail...>::dispatch(idx, alloc, std::forward<Fn>(fn), args...);
        }
    }

    template <typename Fn, typename... Args>
    static void dispatch(Int idx, const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if (idx == Index)
        {
            const Head* head = nullptr;
            if (!alloc->is_empty(Index + StartIdx))
            {
                head = alloc->template get<Head>(Index + StartIdx);
            }

            fn.template stream<Index>(head, args...);
        }
        else {
            PackedDispatcher<StartIdx, Tail...>::dispatch(idx, alloc, std::forward<Fn>(fn), args...);
        }
    }

    template <Int StreamIdx, typename Fn, typename... Args>
    static void dispatch(PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        typedef TypeList<StreamDescr<Head, Index>, Tail...> List;

        typedef typename SelectByIndexTool<StreamIdx, List>::Result::Type StreamType;

        StreamType* head = nullptr;
        if (!alloc->is_empty(StreamIdx + StartIdx))
        {
            head = alloc->template get<StreamType>(StreamIdx + StartIdx);
        }

        fn.template stream<StreamIdx>(head, args...);
    }

    template <Int StreamIdx, typename Fn, typename... Args>
    static void dispatch(const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        typedef TypeList<StreamDescr<Head, Index>, Tail...> List;

        typedef typename SelectByIndexTool<StreamIdx, List>::Result::Type StreamType;

        const StreamType* head = nullptr;
        if (!alloc->is_empty(StreamIdx + StartIdx))
        {
            head = alloc->template get<StreamType>(StreamIdx + StartIdx);
        }

        fn.template stream<StreamIdx>(head, args...);
    }


    template <typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchRtn(Int idx, PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if (idx == Index)
        {
            Head* head = nullptr;
            if (!alloc->is_empty(Index + StartIdx))
            {
                head = alloc->template get<Head>(Index + StartIdx);
            }

            return fn.template stream<Index>(head, args...);
        }
        else {
            return PackedDispatcher<StartIdx, Tail...>::dispatchRtn(idx, alloc, std::forward<Fn>(fn), args...);
        }
    }

    template <typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchRtn(Int idx, const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if (idx == Index)
        {
            const Head* head = nullptr;
            if (!alloc->is_empty(Index + StartIdx))
            {
                head = alloc->template get<Head>(Index + StartIdx);
            }

            return fn.template stream<Index>(head, args...);
        }
        else {
            return PackedDispatcher<StartIdx, Tail...>::dispatchRtn(idx, alloc, std::forward<Fn>(fn), args...);
        }
    }

    template <Int StreamIdx, typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchRtn(PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        typedef TypeList<StreamDescr<Head, Index>, Tail...> List;

        typedef typename SelectByIndexTool<StreamIdx, List>::Result::Type StreamType;

        StreamType* head = nullptr;
        if (!alloc->is_empty(StreamIdx + StartIdx))
        {
            head = alloc->template get<StreamType>(StreamIdx + StartIdx);
        }

        return fn.template stream<StreamIdx>(head, args...);
    }

    template <Int StreamIdx, typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchRtn(const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        typedef TypeList<StreamDescr<Head, Index>, Tail...> List;

        typedef typename SelectByIndexTool<StreamIdx, List>::Result::Type StreamType;

        const StreamType* head = nullptr;
        if (!alloc->is_empty(StreamIdx + StartIdx))
        {
            head = alloc->template get<StreamType>(StreamIdx + StartIdx);
        }

        return fn.template stream<StreamIdx>(head, args...);
    }


    template <typename Fn, typename... Args>
    static void dispatchStatic(Int idx, Fn&& fn, Args&&... args)
    {
        if (idx == Index)
        {
            fn.template stream<Head>(args...);
        }
        else {
            PackedDispatcher<StartIdx, Tail...>::dispatchStatic(idx, std::forward<Fn>(fn), args...);
        }
    }

    template <typename Fn, typename... Args>
    static void dispatchAllStatic(Fn&& fn, Args&&... args)
    {
        Head* head = nullptr;
        fn.template stream<Index>(head, args...);
        PackedDispatcher<StartIdx, Tail...>::dispatchAllStatic(std::forward<Fn>(fn), args...);
    }

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if (!alloc->is_empty(Index + StartIdx))
        {
            Head* head = alloc->template get<Head>(Index + StartIdx);
            fn.template stream<Index>(head, args...);
        }

        PackedDispatcher<StartIdx, Tail...>::dispatchNotEmpty(alloc, std::forward<Fn>(fn), args...);
    }

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if (!alloc->is_empty(Index + StartIdx))
        {
            const Head* head = alloc->template get<Head>(Index + StartIdx);
            fn.template stream<Index>(head, args...);
        }

        PackedDispatcher<StartIdx, Tail...>::dispatchNotEmpty(alloc, std::forward<Fn>(fn), args...);
    }

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(UBigInt streams, PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if ((streams & (1ull << Index)) && !alloc->is_empty(Index + StartIdx))
        {
            Head* head = alloc->template get<Head>(Index + StartIdx);
            fn.template stream<Index>(head, args...);
        }

        PackedDispatcher<StartIdx, Tail...>::dispatchNotEmpty(streams, alloc, std::forward<Fn>(fn), args...);
    }

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(UBigInt streams, const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        if ((streams & (1ull << Index)) && !alloc->is_empty(Index + StartIdx))
        {
            const Head* head = alloc->template get<Head>(Index + StartIdx);
            fn.template stream<Index>(head, args...);
        }

        PackedDispatcher<StartIdx, Tail...>::dispatchNotEmpty(streams, alloc, std::forward<Fn>(fn), args...);
    }


    template <typename Fn, typename... Args>
    static void dispatchAll(PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        Head* head = nullptr;
        if (!alloc->is_empty(Index + StartIdx))
        {
            head = alloc->template get<Head>(Index + StartIdx);
        }

        fn.template stream<Index>(head, args...);

        PackedDispatcher<StartIdx, Tail...>::dispatchAll(alloc, std::forward<Fn>(fn), args...);
    }


    template <typename Fn, typename... Args>
    static void dispatchAll(const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        const Head* head = nullptr;
        if (!alloc->is_empty(Index + StartIdx))
        {
            head = alloc->template get<Head>(Index + StartIdx);
        }

        fn.template stream<Index>(head, args...);

        PackedDispatcher<StartIdx, Tail...>::dispatchAll(alloc, std::forward<Fn>(fn), args...);
    }


    template <typename Fn, typename... Args>
    static void dispatchAll(UBigInt streams, PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        Head* head = nullptr;
        if (!alloc->is_empty(Index + StartIdx) && (streams & (1 << Index)))
        {
            head = alloc->template get<Head>(Index + StartIdx);
        }

        fn.template stream<Index>(head, args...);

        PackedDispatcher<StartIdx, Tail...>::dispatchAll(streams, alloc, std::forward<Fn>(fn), args...);
    }

    template <typename Fn, typename... Args>
    static void dispatchAll(UBigInt streams, const PackedAllocator* alloc, Fn&& fn, Args&&... args)
    {
        Head* head = nullptr;
        if (!alloc->is_empty(Index + StartIdx) && (streams & (1 << Index)))
        {
            head = alloc->template get<Head>(Index + StartIdx);
        }

        fn.template stream<Index>(head, args...);

        PackedDispatcher<StartIdx, Tail...>::dispatchAll(streams, alloc, std::forward<Fn>(fn), args...);
    }

    template <typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchStaticRtn(Int idx, Fn&& fn, Args&&... args)
    {
        if (idx == Index)
        {
            const Head* head = nullptr;
            return fn.template stream<Index>(head, args...);
        }
        else {
            return PackedDispatcher<StartIdx, Tail...>::dispatchStaticRtn(idx, std::forward<Fn>(fn), args...);
        }
    }
};

template <Int StartIdx>
class PackedDispatcher<StartIdx> {
public:
    template <typename Fn, typename... Args>
    static void dispatch(Int idx, PackedAllocator* alloc, Fn&& fn, Args&&...) {
        throw DispatchException(MA_SRC, SBuf()<<"Can't dispatch packed allocator structure: "<<idx);
    }

    template <typename Fn, typename... Args>
    static void dispatch(Int idx, const PackedAllocator* alloc, Fn&& fn, Args&&...) {
        throw DispatchException(MA_SRC, SBuf()<<"Can't dispatch packed allocator structure: "<<idx);
    }

    template <typename Fn, typename... Args>
    static void dispatchStatic(Int idx, Fn&& fn, Args&&...)
    {
        throw DispatchException(MA_SRC, SBuf()<<"Can't dispatch packed allocator structure: "<<idx);
    }

    template <typename Fn, typename... Args>
    static void dispatchAllStatic(Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(const PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(UBigInt, PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static void dispatchNotEmpty(UBigInt, const PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}


    template <typename Fn, typename... Args>
    static void dispatchNotEmptySelected(const PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static void dispatchNotEmptySelected(PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static void dispatchAll(PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static void dispatchAll(const PackedAllocator* alloc, Fn&& fn, Args&&...)
    {}

    template <typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchRtn(Int idx, PackedAllocator* alloc, Fn&& fn, Args&&...) {
        throw DispatchException(MA_SRC, SBuf()<<"Can't dispatch packed allocator structure: "<<idx);
    }

    template <typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchRtn(Int idx, const PackedAllocator* alloc, Fn&& fn, Args&&...) {
        throw DispatchException(MA_SRC, SBuf()<<"Can't dispatch packed allocator structure: "<<idx);
    }

    template <typename Fn, typename... Args>
    static typename std::remove_reference<Fn>::type::ResultType
    dispatchStaticRtn(Int idx, Fn&& fn, Args&&...)
    {
        throw DispatchException(MA_SRC, SBuf()<<"Can't dispatch packed allocator structure: "<<idx);
    }
};




template <typename List, Int Idx = 0> struct PackedDispatchersListBuilder;

template <typename Head, typename... Tail, Int Idx>
struct PackedDispatchersListBuilder<TypeList<Head, Tail...>, Idx> {
     typedef typename MergeLists<
                StreamDescr<Head, Idx>,
                typename PackedDispatchersListBuilder<
                    TypeList<Tail...>,
                    Idx + 1
                >::Type
     >::Result                                                                  Type;
};

template <Int Idx>
struct PackedDispatchersListBuilder<TypeList<>, Idx> {
    typedef TypeList<>                                                          Type;
};

}

#endif
