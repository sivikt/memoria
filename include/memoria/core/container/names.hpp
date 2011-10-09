
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CORE_CONTAINER_NAMES_HPP
#define	_MEMORIA_CORE_CONTAINER_NAMES_HPP


namespace memoria    {

class ContainerCollectionCfgName                    	{};

template <typename PageType, int MaxPageSize = 4096>
struct AbstractAllocatorName {};

template <typename Profile, typename Params> class AbstractAllocatorFactory;

}

#endif
