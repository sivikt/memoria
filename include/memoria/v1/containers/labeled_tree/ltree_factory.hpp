
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memoria/v1/core/types/types.hpp>

#include <memoria/v1/containers/labeled_tree/ltree_names.hpp>

#include <memoria/v1/containers/labeled_tree/container/ltree_c_api.hpp>
#include <memoria/v1/containers/labeled_tree/container/ltree_c_find.hpp>
#include <memoria/v1/containers/labeled_tree/container/ltree_c_insert.hpp>
#include <memoria/v1/containers/labeled_tree/container/ltree_c_update.hpp>
#include <memoria/v1/containers/labeled_tree/container/ltree_c_remove.hpp>
#include <memoria/v1/containers/labeled_tree/container/ltree_c_checks.hpp>

#include <memoria/v1/containers/labeled_tree/iterator/ltree_i_api.hpp>


#include <memoria/v1/containers/seq_dense/seqd_factory.hpp>
#include <memoria/v1/containers/labeled_tree/ltree_walkers.hpp>
#include <memoria/v1/containers/labeled_tree/ltree_tools.hpp>

namespace memoria {
namespace v1 {



template <typename Profile, typename... LabelDescriptors>
struct BTTypes<Profile, v1::LabeledTree<LabelDescriptors...>>: BTTypes<Profile, v1::BT> {

    typedef BTTypes<Profile, v1::BT>                                       Base;

    typedef UBigInt                                                             Value;


    static constexpr Int BitsPerSymbol  = 1;
    static constexpr Int Symbols        = 2;

    typedef TypeList<
                BranchNodeTypes<BranchNode>,
                LeafNodeTypes<LeafNode>
    >                                                                           NodeTypesList;

    typedef TypeList<
                TreeNodeType<LeafNode>,
                TreeNodeType<BranchNode>
    >                                                                           DefaultNodeTypesList;

    using StreamDescriptors = MergeLists<
            StreamTF<
                PkdFSSeq<typename PkdFSSeqTF<1>::Type>,
                FSEBranchStructTF,
                TL<TL<>>
            >,
            StreamTF<
                TL<typename louds::StreamDescriptorsListHelper<LabelDescriptors...>::LeafType>,
                FSEBranchStructTF,
                TL<typename louds::StreamDescriptorsListHelper<LabelDescriptors...>::IdxList>
            >
    >;



    using Metadata = BalancedTreeMetadata<
            typename Base::ID,
            ListSize<StreamDescriptors>::Value
    >;


    using CommonContainerPartsList = MergeLists<
                typename Base::CommonContainerPartsList,

                seq_dense::CtrFindName,
                seq_dense::CtrInsertName,
                seq_dense::CtrRemoveName,

                louds::CtrApiName,
                louds::CtrFindName,
                louds::CtrInsertName,
                louds::CtrUpdateName,
                louds::CtrRemoveName,
                louds::CtrChecksName
    >;

    using FixedLeafContainerPartsList = MergeLists<
                typename Base::FixedLeafContainerPartsList,

                seq_dense::CtrInsertFixedName
    >;

    using VariableLeafContainerPartsList = MergeLists<
                typename Base::VariableLeafContainerPartsList,

                seq_dense::CtrInsertVariableName
    >;


    using IteratorPartsList = MergeLists<
                typename Base::IteratorPartsList,

                seq_dense::IterSelectName,
                seq_dense::IterMiscName,
                seq_dense::IterCountName,
                seq_dense::IterRankName,
                seq_dense::IterSkipName,

                louds::ItrApiName
    >;

    using LabelsTuple = typename louds::LabelsTupleTF<LabelDescriptors...>::Type;

    template <typename Iterator, typename Container>
    struct IteratorCacheFactory {
        typedef v1::louds::LOUDSIteratorCache<Iterator, Container> Type;
    };
};



template <typename Profile, typename... LabelDescriptors, typename T>
class CtrTF<Profile, v1::LabeledTree<LabelDescriptors...>, T>: public CtrTF<Profile, v1::BT, T> {
};


}}