
// Copyright Victor Smirnov 2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_CORE_TYPES_LIST_TUPLE_HPP_
#define MEMORIA_CORE_TYPES_LIST_TUPLE_HPP_

#include<memoria/core/types/types.hpp>

#include<tuple>

namespace memoria {


template <typename T, Int Size>
struct MakeList {
    using Type = MergeLists<
                T,
                typename MakeList<T, Size - 1>::Type
    >;
};

template <typename T>
struct MakeList<T, 0> {
    using Type = TypeList<>;
};

template <typename T> struct MakeTupleH;

template <typename... List>
struct MakeTupleH<TypeList<List...>> {
    using Type = std::tuple<List...>;
};

template <typename T, Int Size>
using MakeTuple = typename MakeTupleH<typename MakeList<T, Size>::Type>::Type;


}

#endif /* TUPLE_HPP_ */