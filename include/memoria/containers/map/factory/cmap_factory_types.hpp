
// Copyright Victor Smirnov 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CONTAINERS_CMAP_FACTORY_TYPES_HPP
#define _MEMORIA_CONTAINERS_CMAP_FACTORY_TYPES_HPP

#include <memoria/prototypes/bt/bt_factory.hpp>
#include <memoria/prototypes/ctr_wrapper/ctrwrapper_factory.hpp>

#include <memoria/containers/map/map_walkers.hpp>
#include <memoria/containers/map/map_tools.hpp>

#include <memoria/containers/map/container/map_c_insert.hpp>
#include <memoria/containers/map/container/map_c_remove.hpp>
#include <memoria/containers/map/container/map_c_api.hpp>

#include <memoria/containers/map/map_iterator.hpp>
#include <memoria/containers/map/iterator/map_i_api.hpp>
#include <memoria/containers/map/iterator/map_i_nav.hpp>

#include <memoria/containers/map/map_names.hpp>

namespace memoria    {





template <typename Profile, Granularity gr>
struct BTTypes<Profile, memoria::CMap<gr> >: public BTTypes<Profile, memoria::BT> {

    typedef BTTypes<Profile, memoria::BT>                   					Base;

    typedef BigInt                                                          	Value;
    typedef TypeList<BigInt>                                                  	KeysList;

    static const Int Indexes                                                	= 1;


    typedef TypeList<
    		LeafNodeTypes<LeafNode>,
    		NonLeafNodeTypes<BranchNode>
    >																			NodeTypesList;

    typedef TypeList<
        	LeafNodeType<LeafNode>,
        	BranchNodeType<BranchNode>
    >																			DefaultNodeTypesList;

    typedef TypeList<
    		StreamDescr<
    			PkdFTreeTF,
    			map::MapTypeTF<Granularity::Bit>::Type,
    			1
    		>
    >																			StreamDescriptors;

    typedef BalancedTreeMetadata<
    		typename Base::ID,
    		ListSize<StreamDescriptors>::Value
    >        																	Metadata;


	typedef typename MergeLists<
				typename Base::ContainerPartsList,
				bt::NodeComprName,
				map::CtrInsertName,
				map::CtrRemoveName,
				map::CtrApiName
	>::Result                                           						ContainerPartsList;


	typedef typename MergeLists<
				typename Base::IteratorPartsList,
				map::ItrApiName,
				map::ItrNavName
	>::Result                                           						IteratorPartsList;


	template <typename Iterator, typename Container>
	struct IteratorCacheFactory {
		typedef map::MapIteratorPrefixCache<Iterator, Container> Type;
	};



    template <typename Types>
    using FindLTWalker 		= map::FindLTWalker<Types>;

    template <typename Types>
    using FindLEWalker 		= map::FindLEWalker<Types>;


    template <typename Types>
    using FindBeginWalker 	= map::FindBeginWalker<Types>;

    template <typename Types>
    using FindEndWalker 	= map::FindEndWalker<Types>;

    template <typename Types>
    using FindRBeginWalker 	= map::FindRBeginWalker<Types>;

    template <typename Types>
    using FindREndWalker 	= map::FindREndWalker<Types>;
};


}

#endif
