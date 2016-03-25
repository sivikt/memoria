
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

#include <memoria/v1/core/types/types.hpp>

#include <memoria/v1/prototypes/bt_tl/bttl_names.hpp>
#include <memoria/v1/prototypes/bt_tl/bttl_tools.hpp>

#include <memoria/v1/core/container/iterator.hpp>
#include <memoria/v1/core/container/macros.hpp>



#include <iostream>

namespace memoria {
namespace v1 {


MEMORIA_V1_ITERATOR_PART_BEGIN(v1::bttl::IteratorMiscName)

    typedef typename Base::Allocator                                            Allocator;
    typedef typename Base::NodeBaseG                                            NodeBaseG;


    typedef typename Base::Container::BranchNodeEntry                               BranchNodeEntry;
    typedef typename Base::Container                                            Container;
    typedef typename Base::Container::Position                                  Position;

    using CtrSizeT  = typename Container::Types::CtrSizeT;
    using Key       = typename Container::Types::Key;
    using Value     = typename Container::Types::Value;

    using LeafDispatcher = typename Container::Types::Pages::LeafDispatcher;

    static const Int Streams                = Container::Types::Streams;
    static const Int SearchableStreams      = Container::Types::SearchableStreams;

    using LeafPrefixRanks = typename Container::Types::LeafPrefixRanks;


public:


    template <typename LeafPath>
    struct ScanFn {
        template <typename Node, typename Fn>
        auto treeNode(const Node* node, Fn&& fn, Int from, CtrSizeT to)
        {
            auto stream = node->template substream<LeafPath>();
            Int stream_size = stream->size();

            Int limit = (to > stream_size) ? stream_size : to;

            fn(stream, from, limit);

            return limit - from;
        }
    };



    template <typename LeafPath, typename Fn>
    CtrSizeT scan(Fn&& fn, CtrSizeT limit = -1)
    {
        auto& self  = this->self();
        auto& cache = self.cache();

        constexpr Int StreamIdx = ListHead<LeafPath>::Value;

        MEMORIA_V1_ASSERT(StreamIdx, ==, self.stream());

        auto size = cache.data_size()[StreamIdx];

        if (limit == -1 || limit > size) {
            limit = size;
        }

        auto pos = cache.data_pos()[StreamIdx];

        CtrSizeT total = 0;

        while (pos < limit)
        {
            auto idx = self.idx();

            Int processed = LeafDispatcher::dispatch(self.leaf(), ScanFn<LeafPath>(), std::forward<Fn>(fn), idx, idx + (limit - pos));

            total += self.skipFw(processed);

            pos = cache.data_pos()[StreamIdx];
        }

        return total;
    }


    void refresh()
    {
        Base::refresh();

        auto& self  = this->self();
        auto& cache = self.cache();

        cache.data_size()[0] = self.ctr().sizes()[0];

        // FIXME: Is it necessary here? Is it correct?
        cache.data_pos()[0] = 0;
        cache.abs_pos()[0]  = 0;
    }

    void refresh_prefixes()
    {
        Base::refresh();
    }


    void checkPrefix() {
        auto tmp = self();

        tmp.refresh();


        MEMORIA_V1_ASSERT(self().cache(), ==, tmp.cache());
    }

    void prepare() {
        Base::prepare();

        auto& self = this->self();
        auto& cache = self.cache();

        cache.data_pos()[0] = 0;
    }


    void init()
    {
        Base::init();

        auto& self = this->self();
        auto& cache = self.cache();

        cache.data_size()[0] = self.ctr().size();
    }

protected:

    void update_leaf_ranks() {}

    void update_leaf_ranks(WalkCmd cmd)
    {
        if (cmd == WalkCmd::LAST_LEAF){
            update_leaf_ranks();
        }
    }

MEMORIA_V1_ITERATOR_PART_END

#define M_TYPE      MEMORIA_V1_ITERATOR_TYPE(v1::bttl::IteratorMiscName)
#define M_PARAMS    MEMORIA_V1_ITERATOR_TEMPLATE_PARAMS


#undef M_TYPE
#undef M_PARAMS

}}