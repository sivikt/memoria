
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_MAP_FACTORY_HPP
#define _MEMORIA_PROTOTYPES_MAP_FACTORY_HPP

#include <memoria/prototypes/bt/bt_factory.hpp>
#include <memoria/prototypes/ctr_wrapper/ctrwrapper_factory.hpp>

#include <memoria/prototypes/bt/walkers/bt_skip_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_find_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_edge_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_select_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_rank_walkers.hpp>

#include <memoria/prototypes/metamap/walkers/metamap_rank_walkers.hpp>
#include <memoria/prototypes/metamap/walkers/metamap_select_walkers.hpp>


#include <memoria/prototypes/bt/packed_adaptors/bt_tree_adaptor.hpp>
#include <memoria/prototypes/metamap/packed_adaptors/metamap_packed_adaptors.hpp>

#include <memoria/prototypes/metamap/metamap_tools.hpp>

#include <memoria/prototypes/metamap/container/metamap_c_insert.hpp>
#include <memoria/prototypes/metamap/container/metamap_c_insert_compr.hpp>
#include <memoria/prototypes/metamap/container/metamap_c_nav.hpp>
#include <memoria/prototypes/metamap/container/metamap_c_remove.hpp>
#include <memoria/prototypes/metamap/container/metamap_c_find.hpp>

#include <memoria/prototypes/metamap/metamap_iterator.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_keys.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_nav.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_entry.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_value.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_value_byref.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_labels.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_find.hpp>
#include <memoria/prototypes/metamap/iterator/metamap_i_misc.hpp>

#include <memoria/prototypes/metamap/metamap_names.hpp>

namespace memoria {



template <
    typename Profile,
    Int Indexes_,
    typename Key_,
    typename Value_,
    typename LabelsList,
    typename HiddenLabelsList
>
struct MetaMapBTTypesBase: public BTTypes<Profile, memoria::BT> {

    typedef BTTypes<Profile, memoria::BT>                                       Base;

    typedef typename TupleBuilder<
            typename metamap::LabelTypeListBuilder<HiddenLabelsList>::Type
    >::Type                                                                     HiddenLabelsTuple;

    typedef typename TupleBuilder<
            typename metamap::LabelTypeListBuilder<LabelsList>::Type
    >::Type                                                                     LabelsTuple;



    typedef TypeList<
            LeafNodeTypes<LeafNode>,
            NonLeafNodeTypes<BranchNode>
    >                                                                           NodeTypesList;

    typedef TypeList<
            TreeNodeType<LeafNode>,
            TreeNodeType<BranchNode>
    >                                                                           DefaultNodeTypesList;

    static const Int Labels                                                     = ListSize<LabelsList>::Value;
    static const Int HiddenLabels                                               = ListSize<HiddenLabelsList>::Value;

    typedef typename IfThenElse<
                    IfTypesEqual<Value_, IDType>::Value,
                    typename Base::ID,
                    Value_
    >::Result                                                                   ValueType;

    static const Int Indexes                                                    = Indexes_;

    using StreamTF = metamap::MetaMapStreamTF<Indexes, Key_, ValueType, HiddenLabelsList, LabelsList>;

    typedef typename StreamTF::Key                                              Key;
    typedef typename StreamTF::Value                                            Value;

    typedef metamap::MetaMapEntry<
                    Indexes,
                    Key,
                    Value,
                    HiddenLabelsTuple,
                    LabelsTuple
    >                                                                           Entry;

    typedef metamap::LabelOffsetProc<LabelsList>                                LabelsOffset;
    typedef metamap::LabelOffsetProc<HiddenLabelsList>                          HiddenLabelsOffset;


    typedef TypeList<StreamTF>                                                  StreamDescriptors;

    typedef BalancedTreeMetadata<
            typename Base::ID,
            ListSize<StreamDescriptors>::Value
    >                                                                           Metadata;


    typedef typename MergeLists<
                typename Base::ContainerPartsList,

                metamap::CtrNavName,
                metamap::CtrRemoveName,
                metamap::CtrFindName
    >::Result                                                                   ContainerPartsList;


    typedef typename MergeLists<
                typename Base::IteratorPartsList,

                metamap::ItrKeysName,
                metamap::ItrNavName,
                metamap::ItrEntryName,
                metamap::ItrLabelsName,
                metamap::ItrFindName,
                metamap::ItrMiscName
    >::Result                                                                   IteratorPartsList;


    template <typename Iterator, typename Container>
    struct IteratorCacheFactory {
        typedef metamap::MetaMapIteratorPrefixCache<Iterator, Container> Type;
    };



    template <typename Types>
    using FindGTWalker          = bt1::FindGTForwardWalker<Types, bt1::DefaultIteratorPrefixFn>;

    template <typename Types>
    using FindGEWalker          = bt1::FindGEForwardWalker<Types, bt1::DefaultIteratorPrefixFn>;

    template <typename Types>
    using FindBackwardWalker    = bt1::FindBackwardWalker<Types, bt1::DefaultIteratorPrefixFn>;


    template <typename Types>
    using SkipForwardWalker     = bt1::SkipForwardWalker<Types, bt1::DefaultIteratorPrefixFn>;

    template <typename Types>
    using SkipBackwardWalker    = bt1::SkipBackwardWalker<Types, bt1::DefaultIteratorPrefixFn>;

    template <typename Types>
    using SelectForwardWalker   = metamap::SelectForwardWalker<Types, bt1::DefaultIteratorPrefixFn>;

    template <typename Types>
    using SelectBackwardWalker  = metamap::SelectBackwardWalker<Types, bt1::DefaultIteratorPrefixFn>;


    template <typename Types>
    using FindBeginWalker       = bt1::FindBeginWalker<Types>;
};







template <
    typename Profile,
    Int Indexes_,
    typename Key_,
    typename Value_,
    typename HiddenLabelsList,
    typename LabelsList
>
struct BTTypes<Profile, memoria::MetaMap<Indexes_, Key_, Value_, LabelsList, HiddenLabelsList>>:
    public MetaMapBTTypesBase<Profile, Indexes_, Key_, Value_, LabelsList, HiddenLabelsList> {

    using Base = MetaMapBTTypesBase<Profile, Indexes_, Key_, Value_, LabelsList, HiddenLabelsList>;


    typedef typename MergeLists<
                    typename Base::ContainerPartsList,
                    bt::NodeNormName,
                    metamap::CtrInsertName
    >::Result                                                                   ContainerPartsList;


    typedef typename MergeLists<
                    typename Base::IteratorPartsList,
                    metamap::ItrValueByRefName
    >::Result                                                                   IteratorPartsList;
};


template <
    typename Profile,
    Int Indexes_,
    typename Key_,
    typename Value_,
    typename HiddenLabelsList,
    typename LabelsList,

    Granularity gr
>
struct BTTypes<Profile, memoria::MetaMap<Indexes_, VLen<gr, Key_>, Value_, LabelsList, HiddenLabelsList>>:
    public MetaMapBTTypesBase<Profile, Indexes_, VLen<gr, Key_>, Value_, LabelsList, HiddenLabelsList> {

    using Base = MetaMapBTTypesBase<Profile, Indexes_, VLen<gr, Key_>, Value_, LabelsList, HiddenLabelsList>;


    typedef typename MergeLists<
                    typename Base::ContainerPartsList,
                    bt::NodeComprName,
                    metamap::CtrInsertComprName
    >::Result                                                                   ContainerPartsList;


    typedef typename MergeLists<
                    typename Base::IteratorPartsList,
                    metamap::ItrValueByRefName
    >::Result                                                                   IteratorPartsList;
};



template <
    typename Profile,
    Int Indexes_,
    typename Key_,
    typename Value_,
    typename HiddenLabelsList,
    typename LabelsList,

    Granularity gr
>
struct BTTypes<Profile, memoria::MetaMap<Indexes_, Key_, VLen<gr, Value_>, LabelsList, HiddenLabelsList>>:
    public MetaMapBTTypesBase<Profile, Indexes_, Key_, VLen<gr, Value_>, LabelsList, HiddenLabelsList> {

    using Base = MetaMapBTTypesBase<Profile, Indexes_, Key_, VLen<gr, Value_>, LabelsList, HiddenLabelsList>;


    typedef typename MergeLists<
                    typename Base::ContainerPartsList,
                    bt::NodeComprName,
                    metamap::CtrInsertComprName
    >::Result                                                                   ContainerPartsList;


    typedef typename MergeLists<
                    typename Base::IteratorPartsList,
                    metamap::ItrValueName
    >::Result                                                                   IteratorPartsList;
};



template <
    typename Profile,
    Int Indexes_,
    typename Key_,
    typename Value_,
    typename HiddenLabelsList,
    typename LabelsList,

    Granularity gr1,
    Granularity gr2
>
struct BTTypes<Profile, memoria::MetaMap<Indexes_, VLen<gr1, Key_>, VLen<gr2, Value_>, LabelsList, HiddenLabelsList>>:
    public MetaMapBTTypesBase<Profile, Indexes_, VLen<gr1, Key_>, VLen<gr2, Value_>, LabelsList, HiddenLabelsList> {

    using Base = MetaMapBTTypesBase<Profile, Indexes_, VLen<gr1, Key_>, VLen<gr2, Value_>, LabelsList, HiddenLabelsList>;


    typedef typename MergeLists<
                    typename Base::ContainerPartsList,
                    bt::NodeComprName,
                    metamap::CtrInsertComprName
    >::Result                                                                   ContainerPartsList;


    typedef typename MergeLists<
                    typename Base::IteratorPartsList,
                    metamap::ItrValueName
    >::Result                                                                   IteratorPartsList;
};




}




#endif
