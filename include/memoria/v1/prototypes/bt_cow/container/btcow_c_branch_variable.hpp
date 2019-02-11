
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

#include <memoria/v1/prototypes/bt/tools/bt_tools.hpp>
#include <memoria/v1/prototypes/bt/bt_macros.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include <memoria/v1/prototypes/bt_cow/btcow_names.hpp>

#include <vector>

namespace memoria {
namespace v1 {

MEMORIA_V1_CONTAINER_PART_BEGIN(btcow::BranchVariableName)

public:
    using typename Base::Types;

protected:
    typedef typename Base::Allocator                                            Allocator;

    typedef typename Base::ID                                                   ID;
    
    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Base::Iterator                                             Iterator;

    using NodeDispatcher    = typename Types::Blocks::NodeDispatcher;
    using LeafDispatcher    = typename Types::Blocks::LeafDispatcher;
    using BranchDispatcher  = typename Types::Blocks::BranchDispatcher;

    typedef typename Base::Metadata                                             Metadata;

    typedef typename Types::BranchNodeEntry                                     BranchNodeEntry;
    typedef typename Types::Position                                            Position;

    typedef typename Types::BlockUpdateMgr                                       BlockUpdateMgr;

    typedef std::function<void (NodeBaseG&, NodeBaseG&)>                        SplitFn;

    static const int32_t Streams = Types::Streams;

public:
    void update_path(const NodeBaseG& node);

protected:
    MEMORIA_V1_DECLARE_NODE_FN(InsertFn, insert);
    void insertToBranchNodeP(NodeBaseG& node, int32_t idx, const BranchNodeEntry& keys, const ID& id);

    NodeBaseG splitPathP(NodeBaseG& node, int32_t split_at);
    NodeBaseG splitP(NodeBaseG& node, SplitFn split_fn);

    MEMORIA_V1_DECLARE_NODE_FN(UpdateNodeFn, updateUp);


    bool updateBranchNode(NodeBaseG& node, int32_t idx, const BranchNodeEntry& entry);

    void updateBranchNodes(NodeBaseG& node, int32_t& idx, const BranchNodeEntry& entry);

    void updateBranchNodesNoBackup(NodeBaseG& node, int32_t idx, const BranchNodeEntry& entry);

    MEMORIA_V1_DECLARE_NODE_FN(TryMergeNodesFn, mergeWith);
    bool tryMergeBranchNodes(NodeBaseG& tgt, NodeBaseG& src);
    bool mergeBranchNodes(NodeBaseG& tgt, NodeBaseG& src);
    bool mergeCurrentBranchNodes(NodeBaseG& tgt, NodeBaseG& src);


MEMORIA_V1_CONTAINER_PART_END


#define M_TYPE      MEMORIA_V1_CONTAINER_TYPE(btcow::BranchVariableName)
#define M_PARAMS    MEMORIA_V1_CONTAINER_TEMPLATE_PARAMS

M_PARAMS
void M_TYPE::insertToBranchNodeP(
        NodeBaseG& node,
        int32_t idx,
        const BranchNodeEntry& sums,
        const ID& id
)
{
    auto& self = this->self();

    self.updateBlockG(node);
    BranchDispatcher::dispatch(node, InsertFn(), idx, sums, id);
    self.updateChildren(node, idx);

    if (!node->is_root())
    {
        NodeBaseG parent = self.getNodeParentForUpdate(node);
        int32_t parent_idx = node->parent_idx();

        auto max = self.max(node);
        self.updateBranchNodes(parent, parent_idx, max);
    }
}




M_PARAMS
typename M_TYPE::NodeBaseG M_TYPE::splitP(NodeBaseG& left_node, SplitFn split_fn)
{
    auto& self = this->self();

    if (left_node->is_root())
    {
        self.newRootP(left_node);
    }
    else {
        self.updateBlockG(left_node);
    }

    NodeBaseG left_parent = self.getNodeParentForUpdate(left_node);

    NodeBaseG right_node = self.createNode(left_node->level(), false, left_node->is_leaf(), left_node->memory_block_size());

    split_fn(left_node, right_node);

    auto left_max  = self.max(left_node);
    auto right_max = self.max(right_node);

    int32_t parent_idx = left_node->parent_idx();

    self.updateBranchNodes(left_parent, parent_idx, left_max);

    BlockUpdateMgr mgr(self);
    mgr.add(left_parent);

    try {
        self.insertToBranchNodeP(left_parent, parent_idx + 1, right_max, right_node->id());
    }
    catch (PackedOOMException& ex)
    {
        mgr.rollback();

        NodeBaseG right_parent = splitPathP(left_parent, parent_idx + 1);

        mgr.add(right_parent);

        try {
            self.insertToBranchNodeP(right_parent, 0, right_max, right_node->id());
        }
        catch (PackedOOMException& ex2)
        {
            mgr.rollback();

            int32_t right_parent_size = self.getNodeSize(right_parent, 0);

            splitPathP(right_parent, right_parent_size / 2);

            self.insertToBranchNodeP(right_parent, 0, right_max, right_node->id());
        }
    }

    return right_node;
}

M_PARAMS
typename M_TYPE::NodeBaseG M_TYPE::splitPathP(NodeBaseG& left_node, int32_t split_at)
{
    auto& self = this->self();

    return splitP(left_node, [&self, split_at](NodeBaseG& left, NodeBaseG& right){
        return self.splitBranchNode(left, right, split_at);
    });
}



M_PARAMS
bool M_TYPE::updateBranchNode(NodeBaseG& node, int32_t idx, const BranchNodeEntry& entry)
{
    auto& self = this->self();

    BlockUpdateMgr mgr(self);

    mgr.add(node);

    try {
        BranchDispatcher::dispatch(node, UpdateNodeFn(), idx, entry);
        return true;
    }
    catch (PackedOOMException ex)
    {
        mgr.rollback();
        return false;
    }
}





M_PARAMS
void M_TYPE::updateBranchNodes(NodeBaseG& node, int32_t& idx, const BranchNodeEntry& entry)
{
    auto& self = this->self();

    self.updateBlockG(node);

    if (!self.updateBranchNode(node, idx, entry))
    {
        int32_t size        = self.getNodeSize(node, 0);
        int32_t split_idx   = size / 2;

        NodeBaseG right = self.splitPathP(node, split_idx);

        if (idx >= split_idx)
        {
            idx -= split_idx;
            node = right;
        }

        bool result = self.updateBranchNode(node, idx, entry);
        MEMORIA_V1_ASSERT_TRUE(result);
    }

    if(!node->is_root())
    {
        int32_t parent_idx = node->parent_idx();
        NodeBaseG parent = self.getNodeParentForUpdate(node);

        auto max = self.max(node);
        self.updateBranchNodes(parent, parent_idx, max);
    }
}






M_PARAMS
void M_TYPE::update_path(const NodeBaseG& node)
{
    auto& self = this->self();

    if (!node->is_root())
    {
        auto entry = self.max(node);

        NodeBaseG parent = self.getNodeParentForUpdate(node);
        int32_t parent_idx = node->parent_idx();
        self.updateBranchNodes(parent, parent_idx, entry);
    }
}



M_PARAMS
void M_TYPE::updateBranchNodesNoBackup(NodeBaseG& node, int32_t idx, const BranchNodeEntry& entry)
{
    auto& self = this->self();

    self.updateBranchNode(node, idx, entry);

    if(!node->is_root())
    {
        int32_t parent_idx = node->parent_idx();
        NodeBaseG parent = self.getNodeParentForUpdate(node);

        self.updateBranchNodesNoBackup(parent, parent_idx, entry);
    }
}


M_PARAMS
bool M_TYPE::tryMergeBranchNodes(NodeBaseG& tgt, NodeBaseG& src)
{
    auto& self = this->self();

    BlockUpdateMgr mgr(self);

    self.updateBlockG(src);
    self.updateBlockG(tgt);

    mgr.add(src);
    mgr.add(tgt);

    try {
        int32_t tgt_size            = self.getNodeSize(tgt, 0);
        NodeBaseG src_parent    = self.getNodeParent(src);
        int32_t parent_idx          = src->parent_idx();

        MEMORIA_V1_ASSERT(parent_idx, >, 0);

        BranchDispatcher::dispatch(src, tgt, TryMergeNodesFn());

        self.updateChildren(tgt, tgt_size);

        auto max = self.max(tgt);

        self.removeNonLeafNodeEntry(src_parent, parent_idx);

        int32_t idx = parent_idx - 1;

        self.updateBranchNodes(src_parent, idx, max);

        self.allocator().removeBlock(src->id(), self.master_name());

        return true;
    }
    catch (PackedOOMException ex)
    {
        mgr.rollback();
    }

    return false;
}


M_PARAMS
bool M_TYPE::mergeBranchNodes(NodeBaseG& tgt, NodeBaseG& src)
{
    auto& self = this->self();

    if (self.isTheSameParent(tgt, src))
    {
        return self.mergeCurrentBranchNodes(tgt, src);
    }
    else
    {
        NodeBaseG tgt_parent = self.getNodeParent(tgt);
        NodeBaseG src_parent = self.getNodeParent(src);

        if (mergeBranchNodes(tgt_parent, src_parent))
        {
            return self.mergeCurrentBranchNodes(tgt, src);
        }
        else
        {
            return false;
        }
    }
}




M_PARAMS
bool M_TYPE::mergeCurrentBranchNodes(NodeBaseG& tgt, NodeBaseG& src)
{
    auto& self = this->self();

    if (self.tryMergeBranchNodes(tgt, src))
    {
        self.removeRedundantRootP(tgt);
        return true;
    }
    else {
        return false;
    }
}


#undef M_TYPE
#undef M_PARAMS

}}
