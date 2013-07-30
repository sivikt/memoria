
// Copyright Victor Smirnov 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_BALANCEDTREE_MODEL_INSERTTOOLS_HPP
#define _MEMORIA_PROTOTYPES_BALANCEDTREE_MODEL_INSERTTOOLS_HPP

#include <memoria/prototypes/balanced_tree/bt_tools.hpp>
#include <memoria/core/container/macros.hpp>

#include <vector>

namespace memoria {

using namespace memoria::balanced_tree;
using namespace memoria::core;

using namespace std;

MEMORIA_CONTAINER_PART_BEGIN(memoria::balanced_tree::InsertToolsName)

    typedef typename Base::Types                                                Types;
    typedef typename Base::Allocator                                            Allocator;

    typedef typename Base::ID                                                   ID;
    
    typedef typename Types::NodeBase                                            NodeBase;
    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Base::TreeNodePage                                         TreeNodePage;
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

    typedef typename Types::PageUpdateMgr										PageUpdateMgr;

    static const Int Indexes                                                    = Types::Indexes;
    static const Int Streams                                                    = Types::Streams;

    typedef typename Base::ISubtreeProvider										ISubtreeProvider;
    typedef typename Base::NonLeafNodeKeyValuePair								NonLeafNodeKeyValuePair;


//    struct InsertSharedData {
//
//    	ISubtreeProvider& provider;
//
//    	Accumulator accumulator;
//
//    	BigInt start;
//    	BigInt end;
//    	BigInt total;
//
//    	BigInt remains;
//
//    	BigInt first_cell_key_count;
//
//    	UBigInt active_streams;
//
//    	InsertSharedData(ISubtreeProvider& provider_):
//    		provider(provider_),
//    		start(0),
//    		end(0),
//    		total(provider_.getTotalKeyCount()),
//    		remains(total),
//    		first_cell_key_count(0),
//    		active_streams(provider.getActiveStreams())
//    	{}
//    };
//
//
//
//    BigInt divide(BigInt op1, BigInt op2)
//    {
//        return (op1 / op2) + ((op1 % op2 == 0) ?  0 : 1);
//    }
//
//    BigInt getSubtreeSize(Int level) const;
//
//    MEMORIA_DECLARE_NODE_FN_RTN(GetCountersFn, getCounters, Accumulator);
//    Accumulator getCounters(const NodeBaseG& node, const Position& from, const Position& count) const;
//    Accumulator getNonLeafCounters(const NodeBaseG& node, const Position& from, const Position& count) const;
//
//    void makeRoom(TreePath& path, Int level, const Position& start, const Position& count) const;
//    void makeRoom(TreePath& path, Int level, Int stream, Int start, Int count) const;
//
//    Accumulator insertSubtree(TreePath& path, Position& idx, ISubtreeProvider& provider);
//
//
//
//    void newRoot(TreePath& path);
//
//
//
//    void insertSubtree(TreePath& path, Int &idx, InsertSharedData& data)
//    {
//        TreePath left_path = path;
//        insertSubtree(left_path, idx, path, idx, 0, data);
//
//        me()->finishPathStep(path, idx);
//
//        me()->addTotalKeyCount(data.provider.getTotalKeyCount());
//    }
//
//
//    void insertInternalSubtree(
//            TreePath& left_path,
//            Int left_idx,
//            TreePath& right_path,
//            Int& right_idx,
//            Int level,
//            InsertSharedData& data);
//
//
//
//    void reindexAndUpdateCounters(NodeBaseG& node, Int from, Int count) const
//    {
//    	auto& self = this->self();
//
//        if (from + count == self.getNodeSize(node, 0))
//        {
//            self.reindexRegion(node, from, from + count);
//        }
//        else {
//            self.reindex(node);
//        }
//    }
//
//    MEMORIA_DECLARE_NODE_FN_RTN(SplitNodeFn, splitTo, Accumulator);
//    Accumulator splitNode(NodeBaseG& src, NodeBaseG& tgt, const Position& split_at) const;
//
//    void fillNodeLeft(TreePath& path, Int level, Int from, Int count, InsertSharedData& data);
//    void prepareNodeFillmentRight(Int level, Int count, InsertSharedData& data);
//
//
//    MEMORIA_DECLARE_NODE_FN(MakeRoomFn, insertSpace);
//    MEMORIA_DECLARE_NODE_FN_RTN(MoveElementsFn, splitTo, Accumulator);
//    MEMORIA_DECLARE_NODE_FN_RTN(IsEmptyFn, isEmpty, bool);
//    MEMORIA_DECLARE_NODE_FN_RTN(IsAfterEndFn, isAfterEnd, bool);
//
//    bool isEmpty(const NodeBaseG& node) {
//    	return NodeDispatcher::dispatchConstRtn(node, IsEmptyFn());
//    }
//
//    bool isAfterEnd(const NodeBaseG& node, const Position& idx, UBigInt active_streams)
//    {
//    	return NodeDispatcher::dispatchConstRtn(node, IsAfterEndFn(), idx, active_streams);
//    }


MEMORIA_CONTAINER_PART_END


#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::balanced_tree::InsertToolsName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS

//
//M_PARAMS
//typename M_TYPE::Accumulator M_TYPE::insertSubtree(TreePath& path, Position& idx, ISubtreeProvider& provider)
//{
//    auto& self = this->self();
//
//	NodeBaseG& leaf = path.leaf();
//
//    Position sizes = provider.getTotalSize();
//
//    if (self.checkCapacities(leaf, sizes))
//    {
//    	leaf.update();
//    	Accumulator acc = provider.insertIntoLeaf(leaf, idx, sizes);
//    	self.updateUp(path, 1, path.leaf().parent_idx(), acc, true);
//    	return acc;
//    }
//    else {
//    	TreePath right = path;
//
//    	if (path.getSize() == 1)
//    	{
//    		self.newRoot(path);
//    		right = path;
//    	}
//
//    	if (!self.isAfterEnd(leaf, idx, provider.getActiveStreams()))
//    	{
//    		self.splitPath(path, right, 0, idx, provider.getActiveStreams());
//    	}
//
//    	leaf.update();
//
//    	Accumulator ls = provider.insertIntoLeaf(leaf, idx);
//    	self.updateUp(path, 1, path.leaf().parent_idx(), ls, true);
//
//    	Int parent_idx = path.leaf().parent_idx() + 1;
//
//    	Position remainder = provider.remainder();
//
//    	if (remainder.gtAny(0))
//    	{
//    		Int path_parent_idx 	= parent_idx;
//    		Int right_parent_idx 	= parent_idx;
//
//    		InsertSharedData data(provider);
//
//    		self.insertInternalSubtree(path, path_parent_idx, right, right_parent_idx, 1, data);
//
//    		return ls + data.accumulator;
//    	}
//    	else {
//    		return ls;
//    	}
//    }
//}
//
//
//
//
//
//
//
//
//
//
//
//
//
//M_PARAMS
//BigInt M_TYPE::getSubtreeSize(Int level) const
//{
//    MEMORIA_ASSERT(level, >=, 0);
//
//    BigInt result = 1;
//
//    for (int c = 1; c < level; c++)
//    {
//        BigInt children_at_level = me()->getMaxKeyCountForNode(false, c == 0, c);
//
//        result *= children_at_level;
//    }
//
//    return result;
//}
//
//
//
//M_PARAMS
//typename M_TYPE::Accumulator M_TYPE::getCounters(const NodeBaseG& node, const Position& from, const Position& count) const
//{
//	return NodeDispatcher::dispatchConstRtn(node, GetCountersFn(), from, count);
//}
//
//M_PARAMS
//typename M_TYPE::Accumulator M_TYPE::getNonLeafCounters(const NodeBaseG& node, const Position& from, const Position& count) const
//{
//	return NonLeafDispatcher::dispatchConstRtn(node, GetCountersFn(), from, count);
//}
//
//
//
//
////// ==================================================  PRIVATE API ================================================== ////
//
//
//
//M_PARAMS
//void M_TYPE::insertInternalSubtree(
//        TreePath& left_path,
//        Int left_idx,
//        TreePath& right_path,
//        Int& right_idx,
//        Int level,
//        InsertSharedData& data)
//{
//    auto& self = this->self();
//
//	//FIXME: check node->level() after deletion;
//
//    BigInt  subtree_size    = self.getSubtreeSize(level);
//
//    BigInt  key_count       = data.total - data.start - data.end;
//
//    NodeBaseG& left_node    = left_path[level].node();
//    NodeBaseG& right_node   = right_path[level].node();
//
//    if (left_node == right_node)
//    {
//        Int node_capacity = self.getNonLeafCapacity(left_node, data.active_streams);
//
//        if (key_count <= subtree_size * node_capacity)
//        {
//            // We have enough free space for all subtrees in the current node
//            BigInt total    = divide(key_count, subtree_size);
//
//            fillNodeLeft(left_path, level, left_idx, total, data);
//
//            right_path.moveRight(level - 1, 0, total);
//
//            right_idx       += total;
//
//            data.remains    = data.total - data.start;
//        }
//        else
//        {
//            // There is no enough free space in current node for all subtrees
//            // split the node and proceed
//
//            //FIXME:
//        	self.splitPath(left_path, right_path, level, Position(left_idx), data.active_streams);
//
//            right_idx = 0;
//
//            insertInternalSubtree(left_path, left_idx, right_path, right_idx, level, data);
//        }
//    }
//    else {
//        Int start_capacity  = self.getNonLeafCapacity(left_node, data.active_streams);
//        Int end_capacity    = self.getNonLeafCapacity(right_node, data.active_streams);
//        Int total_capacity  = start_capacity + end_capacity;
//
//
//        BigInt max_key_count = subtree_size * total_capacity;
//
//        if (key_count <= max_key_count)
//        {
//            // Otherwise fill nodes 'equally'. Each node will have almost
//            // equal free space after processing.
//
//            BigInt total_keys           = divide(key_count, subtree_size);
//
//            Int left_count, right_count;
//
//            if (total_keys <= total_capacity)
//            {
//                Int left_usage  = self.getNodeSize(left_path[level], 0);
//                Int right_usage = self.getNodeSize(right_path[level], 0);
//
//                if (left_usage < right_usage)
//                {
//                    left_count  = start_capacity > total_keys ? total_keys : start_capacity;
//                    right_count = total_keys - left_count;
//                }
//                else {
//                    right_count = end_capacity > total_keys ? total_keys : end_capacity;
//                    left_count  = total_keys - right_count;
//                }
//            }
//            else {
//                left_count  = start_capacity;
//                right_count = end_capacity;
//            }
//
//            fillNodeLeft(left_path, level,   left_idx,  left_count,  data);
//
//            prepareNodeFillmentRight(level, right_count, data);
//
//            data.remains    = data.total - data.start;
//
//            fillNodeLeft(right_path, level, 0, right_count,  data);
//
//            right_idx = right_count;
//        }
//        else
//        {
//            // If there are more keys than these nodes can store,
//            // fill nodes fully
//
//            fillNodeLeft(left_path,   level, left_idx,  start_capacity,  data);
//
//            prepareNodeFillmentRight(level, end_capacity, data);
//
//            right_idx += end_capacity;
//
//            // There is something more to insert.
//            Int parent_right_idx = right_path[level].parent_idx();
//            insertInternalSubtree
//            (
//                    left_path,
//                    left_path[level].parent_idx() + 1,
//                    right_path,
//                    parent_right_idx,
//                    level + 1,
//                    data
//            );
//
//            fillNodeLeft(right_path, level, 0, end_capacity, data);
//        }
//    }
//}
//
//
//
//
//
//
//M_PARAMS
//void M_TYPE::fillNodeLeft(TreePath& path, Int level, Int from, Int count, InsertSharedData& data)
//{
//    auto& self = this->self();
//
//	NodeBaseG& node = path[level].node();
//
//    BigInt subtree_size = me()->getSubtreeSize(level);
//
//    self.makeRoom(path, level, Position(from), Position(count));
//
//    for (Int c = from; c < from + count; c++)
//    {
//    	BigInt requested_size;
//
//    	if (data.first_cell_key_count > 0)
//    	{
//    		requested_size              = data.first_cell_key_count;
//    		data.first_cell_key_count   = 0;
//    	}
//    	else {
//    		requested_size  = data.remains >= subtree_size ? subtree_size : data.remains;
//    	}
//
//    	NonLeafNodeKeyValuePair pair    = data.provider.getKVPair(
//    			data.start,
//    			requested_size,
//    			level
//    	);
//
//    	self.setNonLeafKeys(node, c, pair.keys);
//    	self.setChildID(node, c, pair.value);
//
//    	data.start      += pair.key_count;
//    	data.remains    -= pair.key_count;
//    }
//
//    reindexAndUpdateCounters(node, from, count);
//
//    Accumulator accumulator = self.getNonLeafCounters(node, Position(from), Position(count));
//
//    self.updateParentIfExists(path, level, accumulator);
//
//    VectorAdd(data.accumulator, accumulator);
//}
//
//
//M_PARAMS
//void M_TYPE::prepareNodeFillmentRight(Int level, Int count, InsertSharedData& data)
//{
//    BigInt subtree_size = me()->getSubtreeSize(level);
//
//    if (level > 0)
//    {
//        BigInt total = subtree_size * count;
//
//        if (data.remains >= total)
//        {
//            data.end        += total;
//            data.remains    -= total;
//        }
//        else {
//            BigInt remainder = data.remains - (subtree_size * count - subtree_size);
//
//            data.end        += data.remains;
//            data.remains    = 0;
//
//            data.first_cell_key_count = remainder;
//        }
//    }
//    else {
//        data.end        += count;
//        data.remains    -= count;
//    }
//}
//
//
//
//M_PARAMS
//void M_TYPE::makeRoom(TreePath& path, Int level, const Position& start, const Position& count) const
//{
//	path[level].node().update();
//	NodeDispatcher::dispatch(path[level].node(), MakeRoomFn(), start, count);
//
//	if (level > 0)
//	{
//		path.moveRight(level - 1, start.get(), count.get());
//	}
//}
//
//M_PARAMS
//void M_TYPE::makeRoom(TreePath& path, Int level, Int stream, Int start, Int count) const
//{
//    if (count > 0)
//    {
//        path[level].node().update();
//        NodeDispatcher::dispatch(path[level].node(), MakeRoomFn(), stream, start, count);
//
//        if (level > 0)
//        {
//        	path.moveRight(level - 1, start, count);
//        }
//    }
//}
//
//
//M_PARAMS
//typename M_TYPE::Accumulator M_TYPE::splitNode(NodeBaseG& src, NodeBaseG& tgt, const Position& split_at) const
//{
//	return NodeDispatcher::dispatchRtn(src, tgt, SplitNodeFn(), split_at, Position());
//}
//
//
//
//
//M_PARAMS
//void M_TYPE::newRoot(TreePath& path)
//{
//	auto& self = this->self();
//
//    NodeBaseG& root         = path[path.getSize() - 1].node(); // page == root
//    root.update();
//
//    NodeBaseG new_root      = self.createNode1(root->level() + 1, true, false, root->page_size());
//
//    UBigInt root_active_streams = self.getActiveStreams(root);
//
//    self.layoutNonLeafNode(new_root, root_active_streams);
//
//    self.copyRootMetadata(root, new_root);
//
//    self.root2Node(root);
//
//    path.append(TreePathItem(new_root));
//
//    Accumulator keys = root->is_leaf() ? self.getLeafMaxKeys(root) : self.getMaxKeys(root);
//
//    self.insertNonLeaf(new_root, 0, keys, root->id());
//
//    self.set_root(new_root->id());
//}

#undef M_TYPE
#undef M_PARAMS

} //memoria



#endif