
// Copyright 2012 Victor Smirnov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#pragma once

#include <memoria/v1/core/tools/static_array.hpp>
#include <memoria/v1/core/tools/vector_tuple.hpp>
#include <memoria/v1/core/packed/tools/packed_dispatcher.hpp>

#include <memoria/v1/prototypes/bt/bt_names.hpp>
#include <memoria/v1/core/tools/tuple_dispatcher.hpp>
#include <memoria/v1/core/tools/uuid.hpp>

#include <ostream>
#include <tuple>

namespace memoria {
namespace v1 {

template <typename T> struct FieldFactory;

template<>
struct FieldFactory<UUID> {

    using Type = UUID;

    static void serialize(SerializationData& data, const Type& field)
    {
        FieldFactory<UBigInt>::serialize(data, field.lo());
        FieldFactory<UBigInt>::serialize(data, field.hi());
    }

    static void serialize(SerializationData& data, const Type* field, Int size)
    {
        for (Int c = 0; c < size; c++)
        {
            FieldFactory<UBigInt>::serialize(data, field[c].lo());
            FieldFactory<UBigInt>::serialize(data, field[c].hi());
        }
    }


    static void deserialize(DeserializationData& data, Type& field)
    {
        FieldFactory<UBigInt>::deserialize(data, field.lo());
        FieldFactory<UBigInt>::deserialize(data, field.hi());
    }

    static void deserialize(DeserializationData& data, Type* field, Int size)
    {
        for (Int c = 0; c < size; c++)
        {
            FieldFactory<UBigInt>::deserialize(data, field[c].lo());
            FieldFactory<UBigInt>::deserialize(data, field[c].hi());
        }
    }
};




namespace bt        {




template <typename PkdStructList> struct MakeStreamEntryTL;

template <typename Head, typename... Tail>
struct MakeStreamEntryTL<TL<Head, Tail...>> {
    using Type = AppendItemToList<
            typename PkdStructInputType<Head>::Type,
            typename MakeStreamEntryTL<TL<Tail...>>::Type
    >;
};

template <>
struct MakeStreamEntryTL<TL<>> {
    using Type = TL<>;
};


template <typename List> struct TypeListToTupleH;

template <typename List>
using TypeListToTuple = typename TypeListToTupleH<List>::Type;

template <typename... List>
struct TypeListToTupleH<TL<List...>> {
    using Type = std::tuple<List...>;
};

}
}}