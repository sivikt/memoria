
// Copyright 2017 Victor Smirnov
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

#include <memoria/v1/containers/cow_map/container/cowmap_c_insert.hpp>
#include <memoria/v1/containers/cow_map/container/cowmapm_c_insert.hpp>
#include <memoria/v1/containers/cow_map/container/cowmap_c_remove.hpp>
#include <memoria/v1/containers/cow_map/iterator/cowmap_i_nav.hpp>
#include <memoria/v1/containers/cow_map/iterator/cowmapm_i_nav.hpp>
#include <memoria/v1/containers/cow_map/cowmap_iterator.hpp>
#include <memoria/v1/containers/cow_map/cowmap_names.hpp>
#include <memoria/v1/containers/cow_map/cowmap_tools.hpp>

#include <memoria/v1/prototypes/bt_ss_cow/btsscow_factory.hpp>
#include <memoria/v1/prototypes/ctr_wrapper/ctrwrapper_factory.hpp>

#include <memoria/v1/core/packed/tree/fse/packed_fse_quick_tree.hpp>
#include <memoria/v1/core/packed/tree/fse_max/packed_fse_max_tree.hpp>
#include <memoria/v1/core/packed/tree/vle/packed_vle_quick_tree.hpp>
#include <memoria/v1/core/packed/tree/vle/packed_vle_dense_tree.hpp>
#include <memoria/v1/core/packed/tree/vle_big/packed_vle_bigmax_tree.hpp>
#include <memoria/v1/core/packed/array/packed_fse_array.hpp>
#include <memoria/v1/core/packed/array/packed_vle_dense_array.hpp>
#include <memoria/v1/core/packed/misc/packed_sized_struct.hpp>

#ifdef HAVE_BOOST
#include <memoria/v1/core/tools/bignum/bigint.hpp>
#endif

#include <memoria/v1/core/strings/string.hpp>
#include <memoria/v1/core/strings/string_codec.hpp>

#include <memoria/v1/core/tools/uuid.hpp>

#include <tuple>

namespace memoria {
namespace v1 {


template <
    typename Profile,
    typename Key_,
    typename Value_
>
struct CowMapBTTypesBaseBase: public BTCowTypes<Profile, v1::BTCowSingleStream> {

    using Base = BTCowTypes<Profile, v1::BTCowSingleStream>;

    using Key   = Key_;
    using Value = Value_;

    using Entry = std::tuple<Key, Value>;

    using CommonContainerPartsList = MergeLists<
                typename Base::CommonContainerPartsList,
                v1::cowmap::CtrInsertMaxName,
                v1::cowmap::CtrRemoveName
    >;


    using IteratorPartsList = MergeLists<
                typename Base::IteratorPartsList,
                v1::cowmap::ItrNavMaxName
    >;
};



template <
    typename Profile,
    typename Key,
    typename Value,
    int32_t Special = 0
>
struct CowMapBTTypesBase: public CowMapBTTypesBaseBase<Profile, Key, Value> {

    static_assert(
            IsExternalizable<Key>::Value ,
            "Key type must have either ValueCodec or FieldFactory defined"
    );

    static_assert(
            IsExternalizable<Value>::Value ,
            "Value type must have either ValueCodec or FieldFactory defined"
    );

    using LeafKeyStruct = typename cowmap::CowMapKeyStructTF<Key, HasFieldFactory<Key>::Value>::Type;

    using LeafValueStruct = typename cowmap::CowMapValueStructTF<Value, HasFieldFactory<Value>::Value>::Type;

    using StreamDescriptors = TL<
            StreamTF<
                TL<
                    TL<StreamSize>,
                    TL<LeafKeyStruct>,
                    TL<LeafValueStruct>
                >,
                bt::DefaultBranchStructTF
            >
    >;
};





template <
    typename Profile,
    typename Key_,
    typename Value_
>
struct BTCowTypes<Profile, v1::CowMap<Key_, Value_>>: public CowMapBTTypesBase<Profile, Key_, Value_>{};


template <typename Profile, typename Key, typename Value, typename T>
class CtrTF<Profile, v1::CowMap<Key, Value>, T>: public CtrTF<Profile, v1::BTCowSingleStream, T> {
};



}}
