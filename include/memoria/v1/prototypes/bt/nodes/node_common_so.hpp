
// Copyright 2019 Victor Smirnov
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

#include <memoria/v1/core/types.hpp>

#include <memoria/v1/core/packed/tools/packed_stateful_dispatcher.hpp>

namespace memoria {
namespace v1 {

template <typename CtrT, typename NodeType_>
class NodeCommonSO {
protected:
    CtrT* ctr_;
    NodeType_* node_;
public:
    static constexpr int32_t Streams = NodeType_::Streams;

    using NodeType = NodeType_;
    using Position = typename NodeType_::Position;

    NodeCommonSO(): ctr_(), node_() {}
    NodeCommonSO(CtrT* ctr, NodeType* node):
        ctr_(ctr), node_(node)
    {}

    CtrT* ctr() const {return ctr_;}

    NodeType* node() {return node_;}
    const NodeType* node() const {return node_;}



    void check() const {
        node_->check();
    }

    int32_t size(int32_t stream) const {
        return node_->size(stream);
    }

    auto size_sums() const {
        return node_->size_sums();
    }

    uint64_t active_streams() const {
        return node_->active_streams();
    }

    bool shouldBeMergedWithSiblings() const {
        return node_->shouldBeMergedWithSiblings();
    }


    bool checkCapacities(const Position& sizes) const
    {
        return node_->checkCapacities(sizes);
    }

    template <typename Fn, typename... Args>
    void dispatchAll(Fn&& fn, Args&&... args) const
    {
        NodeType::Dispatcher::dispatchAll(node_->allocator(), std::forward<Fn>(fn), std::forward<Args>(args)...);
    }

    template <typename Fn, typename... Args>
    auto processStreamsStart(Fn&& fn, Args&&... args) {
        return node_->processStreamsStart(std::forward<Fn>(fn), std::forward<Args>(args)...);
    }
};

}
}