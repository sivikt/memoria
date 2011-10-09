
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_MODELS_IDX_MAP_FACTORY_HPP
#define _MEMORIA_MODELS_IDX_MAP_FACTORY_HPP

#include <memoria/prototypes/templates/map.hpp>
#include <memoria/prototypes/templates/tree_map.hpp>


#include <memoria/containers/idx_map/container/find.hpp>
#include <memoria/containers/idx_map/container/insert.hpp>
#include <memoria/containers/idx_map/container/remove.hpp>
#include <memoria/containers/idx_map/container/model_api.hpp>
#include <memoria/containers/idx_map/container/tools.hpp>

#include <memoria/containers/idx_map/pages/parts.hpp>

#include <memoria/containers/idx_map/iterator/tools.hpp>
#include <memoria/containers/idx_map/iterator/model_api.hpp>

#include <memoria/prototypes/btree/btree.hpp>

namespace memoria    {

using namespace memoria::btree;


template <typename Profile>
struct BTreeTypes<Profile, memoria::IdxMap>: public BTreeTypes<Profile, memoria::BTree> {
	typedef BTreeTypes<Profile, memoria::BTree> 							Base;

	typedef BigInt															Value;

	static const bool MapType                                               = MapTypes::Index;

	typedef typename AppendTool<
			typename Base::ContainerPartsList,
			typename TLTool<
			memoria::models::idx_map::RemoveName,
			memoria::models::idx_map::ToolsName,
			memoria::models::idx_map::InsertName,
			memoria::models::idx_map::FindName,
			memoria::models::idx_map::ContainerApiName,

			memoria::models::TreeMapName,
			memoria::models::MapName
			>::List
	>::Result                                                               ContainerPartsList;

	typedef typename AppendTool<
			typename Base::IteratorPartsList,
			typename TLTool<
			memoria::models::idx_map::IteratorToolsName,
			memoria::models::idx_map::IteratorContainerAPIName
			>::List
	>::Result                                                               IteratorPartsList;

};

template <typename Profile, typename T>
class CtrTF<Profile, memoria::IdxMap, T>: public CtrTF<Profile, memoria::BTree, T> {
};

}

#endif
