
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CONTAINERS_STREAM_FACTORY_HPP
#define _MEMORIA_CONTAINERS_STREAM_FACTORY_HPP

#include <memoria/core/container/checker.hpp>
#include <memoria/core/container/collection.hpp>

#include <memoria/containers/root/factory.hpp>
#include <memoria/containers/kvmap/factory.hpp>
#include <memoria/containers/idx_map/factory.hpp>
#include <memoria/containers/array/factory.hpp>


#include "names.hpp"
#include "allocator.hpp"

namespace memoria    {

using namespace memoria::btree;


template <typename Profile>
class ContainerCollectionCfg;

template <typename T>
class ContainerCollectionCfg<StreamProfile<T> > {

    struct StreamContainerCollectionCfg: public BasicContainerCollectionCfg<StreamProfile<T> > {
        typedef BasicContainerCollectionCfg<StreamProfile<T> > Base;

        typedef memoria::StreamAllocator<
        		StreamProfile<T>,
					typename Base::Page,
					typename Base::Transaction
				>  	 															AllocatorType;
    };

public:
    typedef StreamContainerCollectionCfg                      							Types;

};


template <typename Profile>
struct ContainerTypesHelper {
		typedef typename ContainerCollectionCfg<Profile>::Types						ContainerTypesType;

	    typedef typename ContainerName2TypeMapBuilder<
								Profile,
								typename ContainerTypesType::RootNamesList
						 >::Result   												Name2TypeMapList;
};

typedef memoria::StreamAllocator<StreamProfile<>, BasicContainerCollectionCfg<StreamProfile<> >::Page, EmptyType> DefaultStreamAllocator;
typedef ContainerTypesCollection<StreamProfile<> > StreamContainerTypesCollection;
typedef Checker<StreamContainerTypesCollection, DefaultStreamAllocator> StreamContainersChecker;

/*
MEMORIA_EXTERN_BASIC_CONTAINER(StreamContainerTypesCollection, memoria::Root)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::ToolsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::StubsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::ChecksName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::InsertName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::RemoveName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::FindName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::models::kvmap::MapApiName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::models::kvmap::FindName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::models::kvmap::ChecksName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::models::kvmap::InsertName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::models::kvmap::RemoveName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::models::TreeMapName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::models::MapName)

MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::IteratorToolsName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::IteratorWalkName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::IteratorAPIName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::IteratorMultiskipName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Root, memoria::btree::IteratorContainerAPIName)
*/

MEMORIA_EXTERN_BASIC_CONTAINER(StreamContainerTypesCollection, memoria::IdxMap)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::ToolsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::StubsName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::ChecksName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::InsertName)


MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::RemoveName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::FindName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::models::idx_map::RemoveName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::itree::ToolsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::models::idx_map::InsertName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::itree::FindName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::models::idx_map::ContainerApiName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::models::TreeMapName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::models::MapName)

MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::IteratorToolsName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::IteratorWalkName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::IteratorAPIName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::IteratorMultiskipName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::btree::IteratorContainerAPIName)

MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::itree::IteratorToolsName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::IdxMap, memoria::models::idx_map::ItrAPIName)



MEMORIA_EXTERN_BASIC_CONTAINER(StreamContainerTypesCollection, memoria::DefKVMap)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::ToolsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::StubsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::ChecksName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::InsertName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::RemoveName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::FindName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::models::kvmap::FindName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::models::kvmap::ChecksName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::models::kvmap::InsertName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::models::kvmap::RemoveName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::models::kvmap::MapApiName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::models::TreeMapName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::models::MapName)

MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::IteratorToolsName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::IteratorWalkName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::IteratorAPIName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::IteratorMultiskipName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::DefKVMap, memoria::btree::IteratorContainerAPIName)

MEMORIA_EXTERN_BASIC_CONTAINER(StreamContainerTypesCollection, memoria::Array)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::ToolsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::StubsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::ChecksName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::InsertName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::RemoveName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::FindName)

MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::dynvector::ToolsName)
MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::dynvector::RemoveName)

//FIXME CtrPart
//MEMORIA_EXTERN_CTR_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::dynvector::InsertName)

MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::IteratorToolsName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::IteratorWalkName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::IteratorAPIName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::IteratorMultiskipName)
MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::btree::IteratorContainerAPIName)

MEMORIA_EXTERN_ITER_PAPRT(StreamContainerTypesCollection, memoria::Array, memoria::dynvector::IteratorAPIName)

/**/


#if !defined(MEMORIA_DLL) && !defined(MEMORIA_MAIN)

// This is a workaroud. GCC 4.5.1 can't build models without this builder.
extern template class ContainerTypesHelper<StreamProfile<> >;

extern template class memoria::StreamAllocator<StreamProfile<>, BasicContainerCollectionCfg<StreamProfile<> >::Page, EmptyType>;
extern template class ContainerTypesCollection<StreamProfile<> >;
extern template class Checker<StreamContainerTypesCollection, DefaultStreamAllocator>;

#endif

}

#endif
