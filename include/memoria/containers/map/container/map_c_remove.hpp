
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_CONTAINERS_MAP_CTR_REMOVE_HPP
#define _MEMORIA_CONTAINERS_MAP_CTR_REMOVE_HPP


#include <memoria/containers/map/map_names.hpp>
#include <memoria/core/container/container.hpp>
#include <memoria/core/container/macros.hpp>

#include <memoria/core/packed/map/packed_fse_map.hpp>



namespace memoria    {

using namespace memoria::bt;

MEMORIA_CONTAINER_PART_BEGIN(memoria::map::CtrRemoveName)

	typedef typename Base::Types                                                Types;
	typedef typename Base::Allocator                                            Allocator;

	typedef typename Base::ID                                                   ID;

	typedef typename Types::NodeBase                                            NodeBase;
	typedef typename Types::NodeBaseG                                           NodeBaseG;
	typedef typename Base::Iterator                                             Iterator;

	typedef typename Base::NodeDispatcher                                       NodeDispatcher;
	typedef typename Base::RootDispatcher                                       RootDispatcher;
	typedef typename Base::LeafDispatcher                                       LeafDispatcher;
	typedef typename Base::NonLeafDispatcher                                    NonLeafDispatcher;


	typedef typename Base::Key                                                  Key;
	typedef typename Base::Value                                                Value;
	typedef typename Base::Element                                              Element;

	typedef typename Base::Metadata                                             Metadata;

	typedef typename Types::Accumulator                                         Accumulator;
	typedef typename Types::Position 											Position;

	typedef typename Base::TreePath                                             TreePath;
	typedef typename Base::TreePathItem                                         TreePathItem;

	static const Int Streams                                                    = Types::Streams;



	struct RemoveFromLeafFn {
		Accumulator& entry_;

		RemoveFromLeafFn(Accumulator& sums):entry_(sums) {}

		template <Int Idx, typename StreamTypes>
		void stream(PackedFSEMap<StreamTypes>* map, Int idx)
		{
			std::get<Idx>(entry_)[0] = map->tree()->value(0, idx);
			map->remove(idx, idx + 1);
		}

		template <typename Node>
		void treeNode(Node* node, Int idx)
		{
			node->layout(1);
			node->template processStream<0>(*this, idx);
		}
	};

	void removeMapEntry(Iterator& iter, Accumulator& sums)
	{
		auto& self 	= this->self();
		auto& leaf 	= iter.leaf();
		Int& idx	= iter.idx();

		RemoveFromLeafFn fn(sums);

		leaf.update();

		LeafDispatcher::dispatch(leaf, fn, idx);

		self.updateParent(leaf, -fn.entry_);

		self.addTotalKeyCount(Position::create(0, -1));

		self.mergeWithSiblings(leaf, [&](const Position& prev_sizes) {
			idx += prev_sizes[0];
		});

		if (iter.isEnd())
		{
			iter.nextLeaf();
		}

		self.removeRedundantRootP(leaf);
	}




	bool removeMapEntries(Iterator& from, Iterator& to, Accumulator& keys);

MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::map::CtrRemoveName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS

M_PARAMS
bool M_TYPE::removeMapEntries(Iterator& from, Iterator& to, Accumulator& keys)
{
	auto& ctr = self();

	auto& from_node 	= from.leaf();
	Position from_pos 	= Position(from.entry_idx());

	auto& to_node 		= to.leaf();
	Position to_pos 	= Position(to.entry_idx());

	bool result = ctr.removeEntries(from_node, from_pos, to_node, to_pos, keys, true).gtAny(0);

	from.idx() = to.idx() = to_pos.get();

	return result;
}


}

#undef M_TYPE
#undef M_PARAMS


#endif
