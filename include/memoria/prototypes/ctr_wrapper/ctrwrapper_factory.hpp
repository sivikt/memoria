
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_PROTOTYPES_WRAPPER_FACTORY_HPP_
#define MEMORIA_PROTOTYPES_WRAPPER_FACTORY_HPP_

#include <memoria/prototypes/ctr_wrapper/container/ctrwrp_c_base.hpp>


namespace memoria {

template <typename Profile_, typename ContainerTypeSelector>
struct CtrWrapperTypes {

    typedef Profile_                                                            Profile;

    typedef TypeList<
        memoria::btree::AllocatorName
    >                                                                           ContainerPartsList;

    typedef TypeList<>                                                          IteratorPartsList;

    typedef EmptyType                                                           IteratorInterface;

    template <typename Types_>
    struct IterBaseFactory {
        typedef CtrWrapperIteratorBase<Types_>                                  Type;
    };

    template <typename Types_>
    struct CtrBaseFactory {
        typedef CtrWrapperContainerBase<Types_>                                 Type;
    };
};



template <typename Profile_, typename T, typename CtrName>
class CtrTF<Profile_, CtrWrapper<CtrName>, T> {

    typedef CtrTF<Profile_, CtrWrapper<CtrName>, T>                             MyType;

    typedef typename ContainerCollectionCfg<Profile_>::Types::AbstractAllocator Allocator;

public:

    struct Types {
    	typedef CtrName							WrappedCtrName;
        typedef Profile_                        Profile;
        typedef MyType::Allocator               Allocator;

        typedef CtrTypesT<Types>        		CtrTypes;
        typedef IterTypesT<Types>       		IterTypes;
    };

    typedef typename Types::CtrTypes                                            CtrTypes;
    typedef typename Types::IterTypes                                           IterTypes;

    typedef Ctr<CtrTypes>                                                       Type;


};




}


#endif /* WRAPPER_FACTORY_HPP_ */
