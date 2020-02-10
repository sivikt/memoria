
// Copyright 2015 Victor Smirnov
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

#include <memoria/prototypes/bt/tools/bt_tools.hpp>
#include <memoria/prototypes/bt/bt_macros.hpp>
#include <memoria/core/container/macros.hpp>

#include <vector>
#include <algorithm>

namespace memoria {

MEMORIA_V1_CONTAINER_PART_BEGIN(bt::InsertBatchVariableName)
public:
    using Types = typename Base::Types;
    using typename Base::Iterator;

protected:
    typedef typename Base::Allocator                                            Allocator;    
    typedef typename Types::NodeBaseG                                           NodeBaseG;
    using typename Base::BlockID;

    typedef typename Base::Metadata                                             Metadata;

    typedef typename Types::BranchNodeEntry                                     BranchNodeEntry;
    typedef typename Types::Position                                            Position;

    typedef typename Types::BlockUpdateMgr                                       BlockUpdateMgr;

    typedef typename Types::CtrSizeT                                            CtrSizeT;

    using typename Base::TreePathT;


    using Checkpoint    = typename Base::Checkpoint;
    using ILeafProvider = typename Base::ILeafProvider;

public:
    class InsertBatchResult {
        int32_t idx_;
        CtrSizeT subtree_size_;
    public:
        InsertBatchResult(int32_t idx, CtrSizeT size): idx_(idx), subtree_size_(size) {}

        int32_t local_pos() const {return idx_;}
        int32_t idx() const {return idx_;}

        CtrSizeT subtree_size() const {return subtree_size_;}
    };

    MEMORIA_V1_DECLARE_NODE_FN_RTN(InsertChildFn, insert, OpStatus);
    Result<InsertBatchResult> ctr_insert_subtree(
            TreePathT& path,
            size_t level,
            int32_t idx,
            ILeafProvider& provider,
            std::function<Result<NodeBaseG> ()> child_fn,
            bool update_hierarchy
    ) noexcept
    {
        using ResultT = Result<InsertBatchResult>;
        auto& self = this->self();

        NodeBaseG node = path[level];

        int32_t batch_size = 32;

        CtrSizeT provider_size0 = provider.size();

        NodeBaseG last_child{};

        while(batch_size > 0 && provider.size() > 0)
        {
            auto checkpoint = provider.checkpoint();

            BlockUpdateMgr mgr(self);
            MEMORIA_TRY_VOID(mgr.add(node));

            int32_t c;

            OpStatus status{OpStatus::OK};

            for (c = 0; c < batch_size && provider.size() > 0; c++)
            {
                MEMORIA_TRY(child, child_fn());

                if (!child.isSet())
                {
                    return MEMORIA_MAKE_GENERIC_ERROR("Subtree is null");
                }

                BranchNodeEntry sums = self.ctr_get_node_max_keys(child);
                if(isFail(self.branch_dispatcher().dispatch(node, InsertChildFn(), idx + c, sums, child->id()))) {
                    status = OpStatus::FAIL;
                    break;
                }

                last_child = child;
            }

            if (isFail(status))
            {
                if (node->level() > 1)
                {
                    auto res = self.ctr_for_all_ids(node, idx, c, [&, this](const BlockID& id) noexcept -> VoidResult
                    {
                        auto& self = this->self();
                        MEMORIA_TRY_VOID(self.ctr_remove_branch_nodes(id));
                        return VoidResult::of();
                    });
                    MEMORIA_RETURN_IF_ERROR(res);
                }

                provider.rollback(checkpoint);
                mgr.rollback();
                batch_size /= 2;
            }
            else {
                idx += c;
            }
        }

        if (last_child) {
            MEMORIA_TRY_VOID(self.complete_tree_path(path, last_child));
        }

        if (update_hierarchy)
        {
            MEMORIA_TRY_VOID(self.ctr_update_path(path, level));
        }

        return ResultT::of(idx, provider_size0 - provider.size());
    }


    Result<NodeBaseG> ctr_build_subtree(ILeafProvider& provider, int32_t level) noexcept
    {
        using ResultT = Result<NodeBaseG>;
        auto& self = this->self();

        if (provider.size() > 0)
        {
            if (level >= 1)
            {
                MEMORIA_TRY(node, self.ctr_create_node(level, false, false));

                MEMORIA_TRY_VOID(self.ctr_layout_branch_node(node, 0xFF));

                size_t height = level + 1;
                TreePathT path = TreePathT::build(node, height);

                MEMORIA_TRY_VOID(self.ctr_insert_subtree(path, level, 0, provider, [this, level, &provider]() noexcept -> Result<NodeBaseG> {
                    auto& self = this->self();
                    return self.ctr_build_subtree(provider, level - 1);
                }, false));

                return ResultT::of(node);
            }
            else {
                return provider.get_leaf();
            }
        }
        else {
            return ResultT::of();
        }
    }





    class ListLeafProvider: public ILeafProvider {
        NodeBaseG   head_;
        CtrSizeT    size_ = 0;

        MyType&     ctr_;

    public:
        ListLeafProvider(MyType& ctr, NodeBaseG head, CtrSizeT size): head_(head),  size_(size), ctr_(ctr) {}

        virtual CtrSizeT size() const
        {
            return size_;
        }

        virtual Result<NodeBaseG> get_leaf() noexcept
        {
            if (head_.isSet())
            {
                auto node = head_;

                auto res = ctr_.store().getBlock(head_->next_leaf_id(), ctr_.master_name());
                MEMORIA_RETURN_IF_ERROR(res);

                head_ = res.get();
                size_--;
                return node;
            }
            else {
                return MEMORIA_MAKE_GENERIC_ERROR("Leaf List is empty");
            }
        }


        virtual Checkpoint checkpoint() {
            return Checkpoint(head_, size_);
        }


        virtual void rollback(const Checkpoint& checkpoint)
        {
            size_   = checkpoint.size();
            head_   = checkpoint.head();
        }
    };


    Result<InsertBatchResult> ctr_insert_batch_to_node(
            TreePathT& path,
            size_t level,
            int32_t idx,
            ILeafProvider& provider,
            bool update_hierarchy = true
    ) noexcept
    {
        auto& self = this->self();
        return self.ctr_insert_subtree(path, level, idx, provider, [&provider, &level, this]() noexcept -> Result<NodeBaseG> {
            auto& self = this->self();
            return self.ctr_build_subtree(provider, level - 1);
        },
        update_hierarchy);
    }




MEMORIA_V1_CONTAINER_PART_END

#define M_TYPE      MEMORIA_V1_CONTAINER_TYPE(bt::InsertBatchVariableName)
#define M_PARAMS    MEMORIA_V1_CONTAINER_TEMPLATE_PARAMS



#undef M_TYPE
#undef M_PARAMS

}
