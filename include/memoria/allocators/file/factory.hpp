
// Copyright Victor Smirnov 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_ALLOCATORS_FILE_FACTORY_HPP
#define _MEMORIA_ALLOCATORS_FILE_FACTORY_HPP

#include <memoria/containers/root/root_factory.hpp>
#include <memoria/containers/map/map_factory.hpp>
#include <memoria/containers/vector/vctr_factory.hpp>
#include <memoria/containers/vector_map/vmap_factory.hpp>
#include <memoria/containers/seq_dense/seqd_factory.hpp>
#include <memoria/containers/labeled_tree/ltree_factory.hpp>
#include <memoria/containers/wt/wt_factory.hpp>
#include <memoria/containers/vector_tree/vtree_factory.hpp>
#include <memoria/containers/dbl_map/dblmap_factory.hpp>

#include <memoria/core/container/metadata_repository.hpp>

#include "names.hpp"
#include "file_allocator.hpp"

namespace memoria    {


template <typename Profile>
class ContainerCollectionCfg;

template <typename T>
class ContainerCollectionCfg<FileProfile<T> > {
public:
    typedef BasicContainerCollectionCfg<FileProfile<T>, BigInt >               	Types;
};


typedef memoria::FileAllocator<
            FileProfile<>,
            ContainerCollectionCfg<FileProfile<> >::Types::Page
>                                                                       		GenericFileAllocator;

template <typename CtrName>
using FCtrTF = CtrTF<FileProfile<>, CtrName>;


//template <typename CtrName>
//using SCtrTF = CtrTF<SmallProfile<>, CtrName>;
//
//typedef PageID<UBigInt> ID4;
//
//MEMORIA_EXTERN_TREE(BigInt, BigInt,     1);
//MEMORIA_EXTERN_TREE(BigInt, EmptyValue, 1);
//MEMORIA_EXTERN_TREE(BigInt, ID4,        1);
//
//
//MEMORIA_EXTERN_TREE(BigInt, EmptyValue, 2);
//MEMORIA_EXTERN_TREE(BigInt, ID4,        2);

}

#endif