
// Copyright 2019 Victor Smirnov
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

#include <memoria/core/datatypes/traits.hpp>
#include <memoria/core/types/typehash.hpp>

#include <memoria/api/common/ctr_api_btss.hpp>

#include <memoria/core/datatypes/io_vector_traits.hpp>

namespace memoria {

class AllocationMap {};

template <typename Profile>
struct ICtrApiTypes<AllocationMap, Profile> {


    using IOVSchema = TL<
//        TL<
//            ICtrApiSubstream<EmptyType, io::ColumnWise1D>
//        >
    >;
};


template <>
struct TypeHash<AllocationMap>: UInt64Value<
    HashHelper<5663493242560930>
> {};

template <>
struct DataTypeTraits<AllocationMap> {

    using Parameters = TL<>;

    static constexpr bool HasTypeConstructors = false;
    static constexpr bool isSdnDeserializable = false;

    static void create_signature(SBuf& buf, const AllocationMap& obj)
    {
        buf << "AllocationMap";
    }

    static void create_signature(SBuf& buf)
    {
        buf << "AllocationMap";
    }
};

}
