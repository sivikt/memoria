
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_CTRWRAPPER_CTRBASE_HPP
#define _MEMORIA_PROTOTYPES_CTRWRAPPER_CTRBASE_HPP

#include <memoria/core/container/container.hpp>
#include <memoria/core/container/names.hpp>
#include <memoria/prototypes/balanced_tree/baltree_macros.hpp>
#include <memoria/core/types/algo.hpp>
#include <memoria/core/tools/static_array.hpp>

#include <iostream>

namespace memoria       {


MEMORIA_BALTREE_MODEL_BASE_CLASS_NO_CTOR_BEGIN(CtrWrapperContainerBase1)

    typedef TypesType                                                           Types;

	typedef typename Types::WrappedCtrName                                      WrappedCtrName;

    typedef typename Types::Profile                                             Profile;

    typedef typename Base::ID                                                   ID;
    typedef typename Base::Allocator                                            Allocator;
    typedef typename Base::CtrShared                                            CtrShared;

    typedef typename CtrTF<Profile, WrappedCtrName, WrappedCtrName>::Type       WrappedCtr;


private:
    WrappedCtr ctr_;


public:

    CtrWrapperContainerBase1(const CtrInitData& data):
    	Base(data),
    	ctr_(data.owner(Base::CONTAINER_HASH))
    {}

    CtrWrapperContainerBase1(const ThisType& other, Allocator* allocator):
        Base(other, allocator),
        ctr_(other.ctr_, allocator)
    {}

    CtrWrapperContainerBase1(ThisType&& other, Allocator* allocator):
        Base(std::move(other), allocator),
        ctr_(std::move(other.ctr_), allocator)
    {}

    //broken constructor
    CtrWrapperContainerBase1(const ThisType& other):
        Base(other),
        ctr_(other.init_data_.owner(Base::CONTAINER_HASH))
    {}

    CtrWrapperContainerBase1(ThisType&& other):
        Base(std::move(other)),
        ctr_(other.init_data_.owner(Base::CONTAINER_HASH))
    {}

    WrappedCtr& ctr() {
        return ctr_;
    }

    const WrappedCtr& ctr() const {
    	return ctr_;
    }

    void operator=(ThisType&& other)
    {
        Base::operator=(std::move(other));

        ctr_ = std::move(other.ctr_);
    }

    void operator=(const ThisType& other)
    {
        Base::operator=(other);

        ctr_ = other.ctr_;
    }

    void initCtr(Int command)
    {
        ctr_.initCtr(&me()->allocator(), me()->name(), command);

        Base::setCtrShared(NULL);
    }

    void initCtr(const ID& root_id)
    {
        ctr_.initCtr(&me()->allocator(), root_id);

        Base::setCtrShared(NULL);
    }


    static Int initMetadata()
    {
    	Int hash = WrappedCtr::initMetadata();

    	if (Base::getMetadata() == NULL)
    	{
    		MetadataList list;

    		list.push_back(WrappedCtr::getMetadata());

    		Base::setMetadata(new ContainerMetadata(
    									TypeNameFactory<typename Types::ContainerTypeName>::name(),
    									list,
    									Base::CONTAINER_HASH,
    									Base::getContainerInterface()
    						  	  )
    						 );

    		MetadataRepository<typename Types::Profile>::registerMetadata(Base::getMetadata());
    	}

    	return hash;
    }


    virtual ID getRootID(BigInt name)
    {
        return me()->ctr().getRootID(name);
    }


    virtual void setRoot(BigInt name, const ID& root_id)
    {
        me()->ctr().setRoot(name, root_id);
    }

    bool check(void* ptr = NULL)
    {
        return ctr_.check(ptr);
    }

    const CtrShared* shared() const {
        return ctr_.shared();
    }

    CtrShared* shared() {
        return ctr_.shared();
    }


    void setBranchingFactor(Int count)
    {
        typename WrappedCtr::Metadata meta = ctr_.getRootMetadata();
        meta.branching_factor() = count;
        ctr_.setRootMetadata(meta);
    }

    Int getBranchingFactor() const
    {
        return ctr_.getRootMetadata().branching_factor();
    }

MEMORIA_BALTREE_MODEL_BASE_CLASS_END


}

#endif
