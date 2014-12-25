
// Copyright Victor Smirnov 2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef MEMORIA_CORE_TYPES_LIST_LIST_TREE_HPP1_
#define MEMORIA_CORE_TYPES_LIST_LIST_TREE_HPP1_

#include <memoria/core/types/list/typelist.hpp>
#include <memoria/core/types/list/linearize.hpp>
#include <memoria/core/types/list/sublist.hpp>
#include <memoria/core/types/algo/select.hpp>

namespace memoria {



template <typename List, typename Path, Int Depth = 1, Int Ptr = 0> struct LeafCount;

template <
    typename Head,
    typename... Tail,
    Int Idx,
    Int... PathTail,
    Int Depth,
    Int Ptr
>
struct LeafCount<TypeList<Head, Tail...>, IntList<Idx, PathTail...>, Depth, Ptr> {
    static const Int PrefixSize = ListSize<Linearize<TypeList<Head>, Depth>>::Value;
private:
public:
    static const Int Value = PrefixSize + LeafCount<
        TypeList<Tail...>,
        IntList<Idx, PathTail...>,
        Depth,
        Ptr + 1
    >::Value;
};


template <
    typename Head,
    typename... Tail,
    Int Idx,
    Int... PathTail,
    Int Depth
>
struct LeafCount<TypeList<Head, Tail...>, IntList<Idx, PathTail...>, Depth, Idx> {
    static const Int Value = LeafCount<
        Head,
        IntList<PathTail...>, Depth
    >::Value;
};



template <
    typename Head,
    typename... Tail,
    Int Depth,
    Int Idx
>
struct LeafCount<TypeList<Head, Tail...>, IntList<>, Depth, Idx> {
    static const Int Value = LeafCount<
        TypeList<Head, Tail...>,
        IntList<0>, Depth
    >::Value;
};


template <
    typename T,
    Int Depth,
    Int Idx
>
struct LeafCount<T, IntList<>, Depth, Idx> {
    static const Int Value = 0;
};



template <typename List, typename Path, Int Depth = 1, Int Ptr = 0> struct SubtreeLeafCount;


template <
    typename Head,
    typename... Tail,
    Int Idx,
    Int... PathTail,
    Int Depth,
    Int Ptr
>
struct SubtreeLeafCount<TypeList<Head, Tail...>, IntList<Idx, PathTail...>, Depth, Ptr> {
    static const Int Value = SubtreeLeafCount<
        TypeList<Tail...>,
        IntList<Idx, PathTail...>,
        Depth,
        Ptr + 1
    >::Value;
};


template <
    typename Head,
    typename... Tail,
    Int Idx,
    Int... PathTail,
    Int Depth
>
struct SubtreeLeafCount<TypeList<Head, Tail...>, IntList<Idx, PathTail...>, Depth, Idx> {
    static const Int Value = sizeof...(PathTail) > 0 ?
            SubtreeLeafCount<
        Head,
        IntList<PathTail...>, Depth
    >::Value :
    ListSize<Linearize<Head, Depth>>::Value;
};




template <
    typename Head,
    typename... Tail,
    Int Depth,
    Int Idx
>
struct SubtreeLeafCount<TypeList<Head, Tail...>, IntList<>, Depth, Idx> {
    static const Int Value = ListSize<Linearize<TypeList<Head, Tail...>, Depth>>::Value;
};


template <
    typename T,
    Int Depth,
    Int Idx
>
struct SubtreeLeafCount<T, IntList<>, Depth, Idx> {
    static const Int Value = 0;
};


template <typename List, typename Path, Int Depth = 1>
using LeafCountInf = LeafCount<List, Path, Depth>;

template <typename List, typename Path, Int Depth = 1>
using LeafCountSup = IntValue<LeafCount<List, Path, Depth>::Value + SubtreeLeafCount<List, Path, Depth>::Value>;


}

#endif
