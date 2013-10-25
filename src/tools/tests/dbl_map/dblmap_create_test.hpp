
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_DBLMAP_CREATE_TEST_HPP_
#define MEMORIA_TESTS_DBLMAP_CREATE_TEST_HPP_

#include <memoria/memoria.hpp>

#include <memoria/tools/profile_tests.hpp>

#include "dblmap_test_base.hpp"

namespace memoria {

using namespace memoria::vapi;
using namespace std;

template <typename Key, typename Value>
class DblMapCreateTest: public DblMapTestBase<Key, Value> {

    typedef DblMapTestBase<Key, Value>     	 									Base;

    typedef DblMapCreateTest<Key, Value>                                     	MyType;

protected:

    typedef typename Base::Allocator                                            Allocator;
    typedef typename Base::Ctr													Ctr;
    typedef typename Ctr::Iterator												Iterator;

    typedef typename Base::StdDblMap											StdDblMap;

    Int dblmap_check_step_ = 10;

public:

    DblMapCreateTest(): Base("Create")
    {
    	MEMORIA_ADD_TEST_PARAM(dblmap_check_step_);

    	MEMORIA_ADD_TEST_WITH_REPLAY(testCreate, replayCreate);
    }


    void testCreate()
    {
    	Allocator allocator;

    	Ctr ctr(&allocator, CTR_CREATE);

    	this->ctr_name_ = ctr.name();

    	StdDblMap std_map;

    	try {

    		Int check_cnt = 0;

    		for (Int c = 0; c < this->size_; c++)
    		{
    			BigInt key1  = this->key1_ = getRandom(50) + 1;
    			BigInt key2  = this->key2_ = getRandom(1000) + 1;

    			BigInt value = this->value_ = getRandom(100);

    			std_map[key1][key2] = value;

    			auto iter = ctr.create(key1);
    			iter.insert2nd(key2, value);

    			this->out()<<"inserted: "<<key1<<" "<<key2<<" "<<value<<std::endl;

    			allocator.commit();
    			this->checkMap(std_map, ctr);

    			if (check_cnt % dblmap_check_step_ == 0)
    			{
    				this->checkMap(std_map, ctr);
    			}

    			check_cnt++;
    		}

    		this->checkMap(std_map, ctr);
    	}
    	catch (...)
    	{
    		this->dump_name_ = this->Store(allocator);

    		throw;
    	}

    	this->StoreAllocator(allocator, this->getResourcePath("create.dump"));
    }

    void replayCreate()
    {
    	Allocator allocator;

    	this->LoadAllocator(allocator, this->dump_name_);

    	Ctr ctr(&allocator, CTR_FIND, this->ctr_name_);

    	auto iter = ctr.create(this->key1_);
    	iter.insert2nd(this->key2_, this->value_);

    	auto iter2 = ctr.find(5);

    	this->dumpEntries(iter2);

    	iter.dump();
    }

    virtual ~DblMapCreateTest() throw() {}

};




}



#endif

