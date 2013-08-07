
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_CONTAINERS_LOUDS_FACTORY_HPP_
#define MEMORIA_CONTAINERS_LOUDS_FACTORY_HPP_

#include <memoria/core/types/types.hpp>

#include <memoria/containers/louds/louds_names.hpp>

#include <memoria/containers/louds/container/louds_c_api.hpp>
#include <memoria/containers/louds/container/louds_c_find.hpp>
#include <memoria/containers/louds/container/louds_c_insert.hpp>
#include <memoria/containers/louds/container/louds_c_update.hpp>
#include <memoria/containers/louds/container/louds_c_remove.hpp>

#include <memoria/containers/louds/iterator/louds_i_api.hpp>


#include <memoria/containers/seq_dense/seqd_factory.hpp>
#include <memoria/containers/louds/louds_walkers.hpp>
#include <memoria/containers/louds/louds_tools.hpp>

namespace memoria {



template <typename Profile, typename... LabelDescriptors>
struct BTTypes<Profile, memoria::LOUDS<LabelDescriptors...>>: BTTypes<Profile, memoria::Sequence<1, true>> {

    typedef BTTypes<Profile, memoria::Sequence<1, true>>                   		Base;

    typedef UBigInt                                                          	Value;
    typedef TypeList<BigInt>                                                  	KeysList;

    static const Int Indexes                                                	= 3;

    typedef TypeList<
    			NonLeafNodeTypes<TreeMapNode>,
    			LeafNodeTypes<TreeLeafNode>
    >																			NodeTypesList;

    typedef TypeList<
    			LeafNodeType<TreeLeafNode>,
    		    InternalNodeType<TreeMapNode>,
    		    RootNodeType<TreeMapNode>,
    		    RootLeafNodeType<TreeLeafNode>
    >																			DefaultNodeTypesList;


    typedef typename louds::StreamDescriptorsListBuilder<
    		LabelDescriptors...
    >::Type																		StreamDescriptors;

    typedef BalancedTreeMetadata<
    		typename Base::ID,
    		ListSize<StreamDescriptors>::Value
    >        																	Metadata;


	typedef typename MergeLists<
				typename Base::ContainerPartsList,
				memoria::bt::NodeComprName,
				memoria::louds::CtrApiName,
				memoria::louds::CtrFindName,
				memoria::louds::CtrInsertName,
				memoria::louds::CtrUpdateName,
				memoria::louds::CtrRemoveName
	>::Result                                           						ContainerPartsList;


	typedef typename MergeLists<
				typename Base::IteratorPartsList,
				memoria::louds::ItrApiName
	>::Result                                           						IteratorPartsList;

	typedef typename louds::LabelsTupleTF<LabelDescriptors...>::Type			LabelsTuple;


	template <typename Iterator, typename Container>
	struct IteratorCacheFactory {
		typedef memoria::louds::LOUDSIteratorCache<Iterator, Container> Type;
	};



    template <typename Types>
    using FindLTWalker 		= ::memoria::louds::SkipForwardWalker<Types>;


    template <typename Types>
    using RankFWWalker 		= ::memoria::louds::RankFWWalker<Types>;

    template <typename Types>
    using RankBWWalker 		= ::memoria::louds::RankBWWalker<Types>;


    template <typename Types>
    using SelectFwWalker 	= ::memoria::louds::SelectForwardWalker<Types>;

    template <typename Types>
    using SelectBwWalker 	= ::memoria::louds::SelectBackwardWalker<Types>;


    template <typename Types>
    using SkipForwardWalker 	= ::memoria::louds::SkipForwardWalker<Types>;

    template <typename Types>
    using SkipBackwardWalker 	= ::memoria::louds::SkipBackwardWalker<Types>;


    template <typename Types>
    using NextLeafWalker 	 	= ::memoria::bt::NextLeafWalker<Types>;

    template <typename Types>
    using PrevLeafWalker 		= ::memoria::bt::PrevLeafWalker<Types>;

    template <typename Types>
    using FindBeginWalker 		= ::memoria::louds::FindBeginWalker<Types>;

    template <typename Types>
    using FindEndWalker 		= ::memoria::louds::FindEndWalker<Types>;

    template <typename Types>
    using FindRBeginWalker 		= ::memoria::louds::FindRBeginWalker<Types>;

    template <typename Types>
    using FindREndWalker 		= ::memoria::louds::FindREndWalker<Types>;
};



template <typename Profile, typename... LabelDescriptors, typename T>
class CtrTF<Profile, memoria::LOUDS<LabelDescriptors...>, T>: public CtrTF<Profile, memoria::Sequence<1, true>, T> {
};


}


#endif
