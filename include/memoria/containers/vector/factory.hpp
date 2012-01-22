
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_MODELS_ARRAY_FACTORY_HPP
#define _MEMORIA_MODELS_ARRAY_FACTORY_HPP




#include <memoria/containers/idx_map/factory.hpp>

#include <memoria/containers/vector/iterator/model_api.hpp>

#include <memoria/containers/vector/pages/data_page.hpp>
#include <memoria/containers/vector/pages/parts.hpp>

#include <memoria/containers/vector/container/api.hpp>
#include <memoria/containers/vector/container/model_api.hpp>

#include <memoria/containers/vector/names.hpp>
#include <memoria/containers/vector/tools.hpp>

#include <memoria/prototypes/dynvector/dynvector.hpp>

namespace memoria {

template <typename Profile>
struct BTreeTypes<Profile, memoria::Vector>: public BTreeTypes<Profile, memoria::DynVector>  {

	typedef BTreeTypes<Profile, memoria::DynVector> 								Base;

	typedef typename AppendLists<
			typename Base::ContainerPartsList,
			typename TLTool<
				memoria::models::array::ApiName,
				memoria::models::array::ContainerApiName
			>::List
	>::Result                                                               		ContainerPartsList;

	typedef typename AppendLists<
			typename Base::IteratorPartsList,
			typename TLTool<
				memoria::models::array::IteratorContainerAPIName
			>::List
	>::Result                                                               		IteratorPartsList;

	typedef memoria::models::array::CountData                             			CountData;
	typedef memoria::models::array::BufferContentDescriptor<CountData>          	BufferContentDescriptor;

	typedef ArrayData                                                     			Buffer;

	template <Int Size>
	struct DataBlockTypeFactory {
		typedef memoria::array::DynVectorData<Size>                       			Type;
	};
};


template <typename Profile, typename T>
class CtrTF<Profile, memoria::Vector, T>: public CtrTF<Profile, memoria::DynVector, T> {

};

}

#endif