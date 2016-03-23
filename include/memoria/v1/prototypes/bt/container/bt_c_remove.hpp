
// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once


#include <memoria/v1/containers/map/map_names.hpp>
#include <memoria/v1/containers/map/map_tools.hpp>
#include <memoria/v1/core/container/container.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include <vector>

namespace memoria {
namespace v1 {

MEMORIA_V1_CONTAINER_PART_BEGIN(v1::bt::RemoveName)

    typedef typename Base::Types                                                Types;

    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Base::Iterator                                             Iterator;

    using NodeDispatcher    = typename Types::Pages::NodeDispatcher;
    using LeafDispatcher    = typename Types::Pages::LeafDispatcher;
    using BranchDispatcher  = typename Types::Pages::BranchDispatcher;

    typedef typename Types::BranchNodeEntry                                     BranchNodeEntry;
    typedef typename Types::Position                                            Position;

    typedef typename Types::PageUpdateMgr                                       PageUpdateMgr;

protected:
    template <Int Stream>
    void remove_stream_entry(Iterator& iter)
    {
        auto& self = this->self();

        auto result = self.template try_remove_stream_entry<Stream>(iter);

        if (!std::get<0>(result))
        {
            iter.split();

            result = self.template try_remove_stream_entry<Stream>(iter);

            if (!std::get<0>(result))
            {
                throw Exception(MA_SRC, "Second removal attempt failed");
            }

            auto max = self.max(iter.leaf());

            self.update_parent(iter.leaf(), max);
        }
        else {
            auto max = self.max(iter.leaf());

            self.update_parent(iter.leaf(), max);

            auto next = self.getNextNodeP(iter.leaf());

            if (next.isSet())
            {
                self.mergeLeafNodes(iter.leaf(), next, [](const Position&){});
            }

            auto prev = self.getPrevNodeP(iter.leaf());

            if (prev.isSet())
            {
                self.mergeLeafNodes(prev, iter.leaf(), [&](const Position& sizes){
                    iter.idx() += sizes[0];
                    iter.leaf() = prev;
                });
            }
        }
    }



MEMORIA_V1_CONTAINER_PART_END

#define M_TYPE      MEMORIA_V1_CONTAINER_TYPE(v1::bt::RemoveName)
#define M_PARAMS    MEMORIA_V1_CONTAINER_TEMPLATE_PARAMS



#undef M_PARAMS
#undef M_TYPE

}}