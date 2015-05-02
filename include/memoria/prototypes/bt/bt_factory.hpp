
// Copyright Victor Smirnov 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_BALANCEDTREE_FACTORY_HPP
#define _MEMORIA_PROTOTYPES_BALANCEDTREE_FACTORY_HPP

#include <memoria/core/types/type2type.hpp>
#include <memoria/core/tools/idata.hpp>
#include <memoria/core/tools/vector_tuple.hpp>

#include <memoria/core/packed/tree/packed_fse_tree.hpp>
#include <memoria/core/packed/tree/packed_vle_tree.hpp>

#include <memoria/core/packed/sseq/packed_fse_searchable_seq.hpp>

#include <memoria/core/packed/tools/packed_allocator.hpp>

#include <memoria/prototypes/bt/bt_names.hpp>
#include <memoria/prototypes/bt/tools/bt_tools.hpp>
#include <memoria/prototypes/bt/bt_walkers.hpp>

#include <memoria/prototypes/bt/walkers/bt_skip_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_find_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_select_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_misc_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_leaf_walkers.hpp>
#include <memoria/prototypes/bt/walkers/bt_rank_walkers.hpp>

#include <memoria/prototypes/bt/nodes/branch_node.hpp>
#include <memoria/prototypes/bt/nodes/leaf_node.hpp>
#include <memoria/prototypes/bt/nodes/node_dispatcher.hpp>
#include <memoria/prototypes/bt/nodes/tree_metadata.hpp>
#include <memoria/prototypes/bt/nodes/node_list_builder.hpp>

#include <memoria/prototypes/bt/container/bt_c_base.hpp>
#include <memoria/prototypes/bt/container/bt_c_tools.hpp>
#include <memoria/prototypes/bt/container/bt_c_checks.hpp>
#include <memoria/prototypes/bt/container/bt_c_insbatch.hpp>
#include <memoria/prototypes/bt/container/bt_c_insbatch_compr.hpp>
#include <memoria/prototypes/bt/container/bt_c_insert.hpp>
#include <memoria/prototypes/bt/container/bt_c_read.hpp>
#include <memoria/prototypes/bt/container/bt_c_update.hpp>
#include <memoria/prototypes/bt/container/bt_c_nodecompr.hpp>
#include <memoria/prototypes/bt/container/bt_c_nodenorm.hpp>
#include <memoria/prototypes/bt/container/bt_c_remtools.hpp>
#include <memoria/prototypes/bt/container/bt_c_rembatch.hpp>
#include <memoria/prototypes/bt/container/bt_c_find.hpp>
#include <memoria/prototypes/bt/container/bt_c_walk.hpp>
#include <memoria/prototypes/bt/container/bt_c_allocator.hpp>
#include <memoria/prototypes/bt/container/bt_c_remove.hpp>

#include <memoria/prototypes/bt/bt_iterator.hpp>

#include <memoria/core/types/typelist.hpp>
#include <memoria/core/tools/assert.hpp>


namespace memoria    {

using memoria::bt::WalkerTypes;

template <typename Profile_, typename ContainerTypeSelector>
struct BTTypes {

    typedef Profile_                                                            Profile;

    using CtrSizeT = BigInt;

    typedef TypeList<
            bt::AllocatorName,
            bt::ToolsName,
            bt::ChecksName,
            bt::InsertBatchName,
            bt::InsertName,
            bt::RemoveToolsName,
            bt::RemoveBatchName,
            bt::RemoveName,
            bt::FindName,
            bt::ReadName,
            bt::UpdateName,
            bt::WalkName
    >                                                                           ContainerPartsList;
    
    typedef TypeList<
            bt::IteratorAPIName,
            bt::IteratorFindName,
            bt::IteratorSelectName,
            bt::IteratorRankName,
            bt::IteratorSkipName,
            bt::IteratorLeafName
    >                                                                           IteratorPartsList;

    typedef EmptyType                                                           ContainerInterface;
    typedef EmptyType                                                           IteratorInterface;
    typedef EmptyType                                                           IteratorData;


    typedef typename ContainerCollectionCfg<Profile_>::Types::AbstractAllocator Allocator;
    typedef typename Allocator::ID                                              ID;


    typedef TypeList<
            BranchNodeTypes<BranchNode>,
            LeafNodeTypes<LeafNode>
    >                                                                           NodeTypesList;

    typedef TypeList<
            TreeNodeType<BranchNode>
    >                                                                           DefaultBranchNodeTypesList;

    typedef TypeList<
            TreeNodeType<LeafNode>
    >                                                                           DefaultLeafNodeTypesList;
    //FIXME DefaultNodeTypesList is not used anymore

    typedef TypeList<>                                                          StreamDescriptors;

    template <
        typename Types_
    >
    struct CtrBaseFactory {
        typedef bt::BTreeCtrBase<Types_>                    Type;
    };

    template <
        typename Types_
    >
    struct IterBaseFactory {
        typedef BTIteratorBase<Types_>                      Type;
    };


    template <typename Iterator, typename Container>
    struct IteratorCacheFactory {
        typedef ::memoria::bt::BTree2IteratorPrefixCache<Iterator, Container>   Type;
    };


    static const Int MAIN_STREAM                                                = 0;


    typedef EmptyType1                                                          DataSource;
    typedef EmptyType2                                                          DataTarget;

    typedef EmptyType1                                                          Source;
    typedef EmptyType2                                                          Target;

    typedef EmptyType                                                           Entropy;


    template <typename Types, typename LeafPath>
    using FindGTForwardWalker          = bt::FindGTForwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using FindGTBackwardWalker         = bt::FindGTBackwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using FindGEForwardWalker          = bt::FindGEForwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using FindGEBackwardWalker         = bt::FindGEBackwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using SkipForwardWalker     = bt::SkipForwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using SkipBackwardWalker    = bt::SkipBackwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using SelectForwardWalker   = bt::SelectForwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using SelectBackwardWalker  = bt::SelectBackwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using RankForwardWalker   = bt::RankForwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using RankBackwardWalker  = bt::RankBackwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types>
    using NextLeafWalker        = bt::ForwardLeafWalker<Types>;

    template <typename Types>
    using PrevLeafWalker        = bt::BackwardLeafWalker<Types>;

    template <typename Types, typename LeafPath>
    using NextLeafMutistreamWalker 	= bt::SkipForwardWalker2<WalkerTypes<Types, LeafPath>>;

    template <typename Types, typename LeafPath>
    using PrevLeafMutistreamWalker	= bt::SkipBackwardWalker2<WalkerTypes<Types, LeafPath>>;
};


template <
        typename Profile,
        typename ContainerTypeName_
>
class CtrTF<Profile, memoria::BT, ContainerTypeName_> {

    typedef CtrTF<Profile, memoria::BT, ContainerTypeName_>                     MyType;

public:

    typedef BTTypes<Profile, ContainerTypeName_>                                ContainerTypes;

    
    typedef typename ContainerTypes::Allocator::Page::ID                        ID;

    typedef typename ContainerTypes::Value                                      Value;

    static const Int Streams = ListSize<typename ContainerTypes::StreamDescriptors>::Value;


    typedef typename bt::TupleBuilder<
                typename bt::IteratorPrefixListBuilder<
                    typename ContainerTypes::StreamDescriptors
                >::Type
    >::Type                                                                     IteratorPrefix_;

    using Position_ = core::StaticVector<typename ContainerTypes::CtrSizeT, Streams>;
    using Page      = typename ContainerTypes::Allocator::Page;

    using NodePageBase0     = TreeNodeBase<typename ContainerTypes::Metadata, Page>;
    using NodePageBase0G    = PageGuard<NodePageBase0, typename ContainerTypes::Allocator>;
    using StreamDescriptors = typename ContainerTypes::StreamDescriptors;

    using BranchStreamsStructList 	= typename PackedBranchStructListBuilder<StreamDescriptors>::StructList;

    using LeafStreamsStructList 	= typename PackedLeafStructListBuilder<StreamDescriptors>::StructList;
    using StreamsInputTypeList 		= typename PackedLeafStructListBuilder<StreamDescriptors>::StreamInputList;

    using IteratorAccumulator = typename TupleBuilder<
    			Linearize<
    				typename IteratorAccumulatorListBuilder<StreamDescriptors>::AccumTuple
                >
    >::Type;

    using LeafRangeOffsetList = Linearize<
    				typename IteratorAccumulatorListBuilder<StreamDescriptors>::RangeOffsetList
    >;

    using LeafRangeList = Linearize<
    				typename IteratorAccumulatorListBuilder<StreamDescriptors>::IndexRangeList,
    				2
    >;


    using Accumulator_ = typename bt::TupleBuilder<
                    typename bt::AccumulatorListBuilder<
                        typename ContainerTypes::StreamDescriptors
                    >::Type
    >::Type;


    struct NodeTypesBase: ContainerTypes {
        using NodeBase  = Page;
        using Name      = ContainerTypeName_;
        using Metadata  = typename ContainerTypes::Metadata;
        using ID        = typename MyType::ID;

        using Accumulator   = Accumulator_;
        using Position      = Position_;

        using LeafStreamsStructList 	= typename MyType::LeafStreamsStructList;
        using BranchStreamsStructList 	= typename MyType::BranchStreamsStructList;
        using IteratorAccumulator 		= typename MyType::IteratorAccumulator;
        using StreamsInputTypeList		= typename MyType::StreamsInputTypeList;
    };

    struct BranchNodeTypes: NodeTypesBase {

    };

    struct LeafNodeTypes: NodeTypesBase {
    };

    struct DispatcherTypes
    {
        using NodeTypesList = typename ContainerTypes::NodeTypesList;

        using DefaultBranchNodeTypesList    = typename ContainerTypes::DefaultBranchNodeTypesList;
        using DefaultLeafNodeTypesList      = typename ContainerTypes::DefaultLeafNodeTypesList;

        using BranchNodeTypes   = typename MyType::BranchNodeTypes;
        using LeafNodeTypes     = typename MyType::LeafNodeTypes;

        using NodeBaseG         = NodePageBase0G;
    };

    typedef bt::BTreeDispatchers<DispatcherTypes>                              PageDispatchers;


public:
    struct Types: ContainerTypes
    {
        typedef ContainerTypeName_                                              ContainerTypeName;
        typedef typename MyType::PageDispatchers                                Pages;

        typedef typename ContainerTypes::Allocator                              Allocator;
        typedef typename ContainerTypes::Metadata                               Metadata;

        typedef NodePageBase0                                                   NodeBase;
        typedef NodePageBase0G                                                  NodeBaseG;

        typedef typename ContainerTypes::ContainerPartsList                     CtrList;
        typedef typename ContainerTypes::IteratorPartsList                      IterList;

        // FIXME Refactor BTree hierarchy
        // Use container types as base definitions
        typedef BTCtrTypes<Types>                                               CtrTypes;
        typedef BTIterTypes<Types>                                              IterTypes;

        static const Int Streams                                                = MyType::Streams;

        typedef Accumulator_                                                    Accumulator;
        typedef IteratorPrefix_                                                 IteratorPrefix;

        typedef Position_                                                       Position;

        typedef PageUpdateManager<CtrTypes>                                     PageUpdateMgr;

        using LeafStreamsStructList 	= typename MyType::LeafStreamsStructList;
        using StreamsInputTypeList 		= typename MyType::StreamsInputTypeList;

        using BranchStreamsStructList 	= typename MyType::BranchStreamsStructList;

        using IteratorAccumulator 		= typename MyType::IteratorAccumulator;
        using LeafRangeOffsetList 		= typename MyType::LeafRangeOffsetList;
        using LeafRangeList				= typename MyType::LeafRangeList;

        template <typename LeafPath>
        using TargetType = typename PackedStructValueTypeH<LeafStreamsStructList, LeafPath>::Type;

        template <Int Stream>
        using StreamInputTuple = TypeListToTuple<typename Select<Stream, StreamsInputTypeList>::Result>;

        template <Int Stream>
        using InputTupleAdapter = StreamTupleHelper<StreamInputTuple<Stream>>;
    };

    typedef typename Types::CtrTypes                                            CtrTypes;
    typedef Ctr<CtrTypes>                                                       Type;
};



}

#endif
