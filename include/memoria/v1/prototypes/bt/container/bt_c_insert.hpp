
// Copyright Victor Smirnov 2011+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memoria/v1/prototypes/bt/tools/bt_tools.hpp>
#include <memoria/v1/prototypes/bt/bt_macros.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include <vector>

namespace memoria {
namespace v1 {

using namespace v1::bt;
using namespace v1::core;

using namespace std;

MEMORIA_CONTAINER_PART_BEGIN(v1::bt::InsertName)

    typedef typename Base::Types                                                Types;
    typedef typename Base::Allocator                                            Allocator;

    typedef typename Base::ID                                                   ID;
    
    typedef typename Types::NodeBase                                            NodeBase;
    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Base::Iterator                                             Iterator;

    using NodeDispatcher    = typename Types::Pages::NodeDispatcher;
    using LeafDispatcher    = typename Types::Pages::LeafDispatcher;
    using BranchDispatcher  = typename Types::Pages::BranchDispatcher;

    typedef typename Base::Metadata                                             Metadata;

    typedef typename Types::BranchNodeEntry                                     BranchNodeEntry;
    typedef typename Types::Position                                            Position;

    typedef typename Types::PageUpdateMgr                                       PageUpdateMgr;

    template <Int Stream, typename Entry>
    SplitStatus insert_stream_entry(Iterator& iter, const Entry& entry)
    {
        auto& self = this->self();

        auto result = self.template try_insert_stream_entry<Stream>(iter, entry);

        SplitStatus split_status;

        if (!std::get<0>(result))
        {
            split_status = iter.split();

            result = self.template try_insert_stream_entry<Stream>(iter, entry);

            if (!std::get<0>(result))
            {
                throw Exception(MA_SRC, "Second insertion attempt failed");
            }
        }
        else {
            split_status = SplitStatus::NONE;
        }

        auto max = self.max(iter.leaf());

        self.update_parent(iter.leaf(), max);

        return split_status;
    }




MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(v1::bt::InsertName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS


#undef M_TYPE
#undef M_PARAMS

}}