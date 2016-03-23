
// Copyright 2013 Victor Smirnov
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


#include <memoria/v1/containers/wt/wt_names.hpp>
#include <memoria/v1/containers/wt/wt_tools.hpp>
#include <memoria/v1/containers/wt/wt_iterator.hpp>
#include <memoria/v1/containers/wt/wt_walkers.hpp>

#include <memoria/v1/containers/wt/container/wt_c_base.hpp>
#include <memoria/v1/containers/wt/container/wt_c_api.hpp>
#include <memoria/v1/containers/wt/container/wt_c_ctree.hpp>

#include <memoria/v1/containers/wt/iterator/wt_i_api.hpp>

#include <memoria/v1/containers/labeled_tree/ltree_factory.hpp>
#include <memoria/v1/containers/seq_dense/seqd_factory.hpp>

#include <memoria/v1/prototypes/composite/comp_factory.hpp>

#include <memoria/v1/containers/wt/factory/wt_lt_factory.hpp>


namespace memoria {
namespace v1 {



template <typename Profile_>
struct CompositeTypes<Profile_, WT>: public CompositeTypes<Profile_, Composite> {

    typedef WT                                                                  ContainerTypeName;

    typedef CompositeTypes<Profile_, Composite>                                 Base;

    using CtrList = MergeLists<
            typename Base::ContainerPartsList,

            wt::CtrApiName,
            wt::CtrCTreeName
    >;

    using IterList = MergeLists<
            typename Base::IteratorPartsList,

            wt::ItrApiName
    >;


    template <typename Types_>
    struct CtrBaseFactory {
        typedef wt::WTCtrBase<Types_>                                           Type;
    };
};


template <typename Profile_, typename T>
class CtrTF<Profile_, WT, T> {

    typedef CtrTF<Profile_, WT, T>                                              MyType;

    typedef typename ContainerCollectionCfg<Profile_>::Types::AbstractAllocator Allocator;

    typedef CompositeTypes<Profile_, WT>                                        ContainerTypes;

public:

    struct Types: public ContainerTypes
    {
        typedef Profile_                                        Profile;
        typedef MyType::Allocator                               Allocator;

        typedef WTCtrTypes<Types>                               CtrTypes;
        typedef WTIterTypes<Types>                              IterTypes;
    };

    typedef typename Types::CtrTypes                                            CtrTypes;
    typedef typename Types::IterTypes                                           IterTypes;

    typedef Ctr<CtrTypes>                                                       Type;
};


}}