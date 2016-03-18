
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_CONTAINER_vctr_C_REMOVE_HPP
#define _MEMORIA_CONTAINER_vctr_C_REMOVE_HPP


#include <memoria/containers/vector/vctr_names.hpp>
#include <memoria/core/container/container.hpp>
#include <memoria/core/container/macros.hpp>



namespace memoria    {

using namespace memoria::bt;

MEMORIA_CONTAINER_PART_BEGIN(memoria::mvector::CtrRemoveName)

public:
	using typename Base::Types;

protected:


MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::mvector::CtrRemoveName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS


}

#undef M_TYPE
#undef M_PARAMS


#endif
