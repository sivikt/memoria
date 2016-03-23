
// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once


#include <memoria/v1/prototypes/bt_tl/bttl_names.hpp>
#include <memoria/v1/core/container/container.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include <memoria/v1/prototypes/bt_tl/bttl_tools.hpp>


#include <vector>

namespace memoria {

MEMORIA_CONTAINER_PART_BEGIN(memoria::bttl::LeafCommonName)
public:
    using Types             = typename Base::Types;
    using Iterator          = typename Base::Iterator;

protected:
    using NodeBaseG         = typename Types::NodeBaseG;
    using NodeDispatcher    = typename Types::Pages::NodeDispatcher;
    using LeafDispatcher    = typename Types::Pages::LeafDispatcher;
    using BranchDispatcher  = typename Types::Pages::BranchDispatcher;

    using Key               = typename Types::Key;
    using Value             = typename Types::Value;
    using CtrSizeT          = typename Types::CtrSizeT;

    using BranchNodeEntry   = typename Types::BranchNodeEntry;
    using Position          = typename Types::Position;

    static const Int Streams = Types::Streams;

    using PageUpdateMgt     = typename Types::PageUpdateMgr;


    bool isAtTheEnd2(const NodeBaseG& leaf, const Position& pos) const
    {
        auto sizes = self().getNodeSizes(leaf);

        return pos.sum() >= sizes.sum();
    }

MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::bttl::LeafCommonName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS



#undef M_PARAMS
#undef M_TYPE

}
