
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_CONTAINER_vctr_C_FIND_HPP
#define _MEMORIA_CONTAINER_vctr_C_FIND_HPP


#include <memoria/containers/vector/vctr_names.hpp>

#include <memoria/core/container/container.hpp>
#include <memoria/core/container/macros.hpp>



namespace memoria    {

using namespace memoria::bt;

MEMORIA_CONTAINER_PART_BEGIN(memoria::mvector::CtrFindName)

    using typename Base::Types;

    using typename Base::NodeBaseG;
    using typename Base::IteratorPtr;

    using typename Base::NodeDispatcher;
    using typename Base::LeafDispatcher;
    using typename Base::BranchDispatcher;
    using typename Base::Position;
    using typename Base::BranchNodeEntry;
    using typename Base::PageUpdateMgr;
    using typename Base::CtrSizeT;

    using Value = typename Types::Value;

//    Iterator Begin() {
//      return self().seek(0);
//    }
//
//    Iterator End()
//    {
//        auto& self = this->self();
//        return self.seek(self.size());
//    }

    IteratorPtr RBegin()
    {
        auto& self  = this->self();
        auto size   = self.size();

        if (size > 0)
        {
            return self.seek(size - 1);
        }
        else {
            return self.seek(size);
        }
    }

    IteratorPtr REnd()
    {
        auto& self  = this->self();
        auto size   = self.size();

        auto iter   = self.Begin();

        if (size > 0)
        {
            iter->prev();
        }

        return iter;
    }

MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::mvector::CtrFindName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS


#undef M_TYPE
#undef M_PARAMS

}


#endif
