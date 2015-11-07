// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_BTTL_REMOVAL_TEST_HPP_
#define MEMORIA_TESTS_BTTL_REMOVAL_TEST_HPP_

#include <memoria/memoria.hpp>

#include <memoria/tools/tests.hpp>
#include <memoria/tools/tools.hpp>

#include "bttl_test_base.hpp"

#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>

namespace memoria {

template <
    typename CtrName,
	typename AllocatorT 	= SmallInMemAllocator,
	typename ProfileT		= SmallProfile<>
>
class BTTLRemovalTest;

template <
	Int Levels,
	PackedSizeType SizeType,
	typename AllocatorT,
	typename ProfileT
>
class BTTLRemovalTest<BTTLTestCtr<Levels, SizeType>, AllocatorT, ProfileT>: public BTTLTestBase<BTTLTestCtr<Levels, SizeType>, AllocatorT, ProfileT> {

	using CtrName = BTTLTestCtr<Levels, SizeType>;

    using Base 	 = BTTLTestBase<CtrName, AllocatorT, ProfileT>;
    using MyType = BTTLRemovalTest<CtrName, AllocatorT, ProfileT>;

    using Allocator 	= typename Base::Allocator;
    using AllocatorSPtr = typename Base::AllocatorSPtr;
    using Ctr 			= typename Base::Ctr;

    using DetInputProvider  	= bttl::DeterministicDataInputProvider<Ctr>;
    using RngInputProvider  	= bttl::RandomDataInputProvider<Ctr, RngInt>;

    using Rng 			 = typename RngInputProvider::Rng;

    using CtrSizesT 	 = typename Ctr::Types::Position;
    using CtrSizeT 	 	 = typename Ctr::Types::CtrSizeT;

    static const Int Streams = Ctr::Types::Streams;

	Int level  = -1;


	CtrSizeT 	length_;
	CtrSizesT	removal_pos_;
	BigInt		ctr_name_;

	Int level_;

public:

    BTTLRemovalTest(String name):
    	Base(name)
    {

    	MEMORIA_ADD_TEST_PARAM(level);

    	MEMORIA_ADD_TEST_PARAM(length_)->state();
    	MEMORIA_ADD_TEST_PARAM(removal_pos_)->state();
    	MEMORIA_ADD_TEST_PARAM(ctr_name_)->state();
    	MEMORIA_ADD_TEST_PARAM(level_)->state();

    	MEMORIA_ADD_TEST_WITH_REPLAY(testRemove, replayRemove);
    }

    virtual ~BTTLRemovalTest() throw () {}

    virtual void smokeCoverage(Int scale)
    {
    	this->size 			= 10000  * scale;
    	this->iterations	= 1;
    }

    virtual void smallCoverage(Int scale)
    {
    	this->size 			= 100000  * scale;
    	this->iterations	= 3;
    }

    virtual void normalCoverage(Int scale)
    {
    	this->size 			= 100000 * scale;
    	this->iterations	= 50;
    }

    virtual void largeCoverage(Int scale)
    {
    	this->size 			= 1000000 * scale;
    	this->iterations	= 10;
    }

    void createAllocator(AllocatorSPtr& allocator)
    {
    	allocator = std::make_shared<Allocator>();
    	allocator->mem_limit() = this->hard_memlimit_;
    }



    void testRemovalStep(Ctr& ctr)
    {
    	auto iter = ctr.seek(removal_pos_[0]);

    	for (Int s = 1; s <= level_; s++) {
    		iter.toData(removal_pos_[s]);
    	}

    	auto sizes_before = ctr.sizes();

    	this->out()<<"Remove "<<length_<<" elements data at: "<<removal_pos_<<" size: "<<sizes_before<<endl;

    	iter.remove_subtrees(length_);

    	auto sizes_after = ctr.sizes();
    	auto ctr_totals = ctr.total_counts();

    	this->out()<<"Totals: "<<ctr_totals<<" "<<sizes_after<<endl;
    	AssertEQ(MA_SRC, ctr_totals, sizes_after);

    	this->checkAllocator(MA_SRC, "Remove: Container Check Failed");

    	this->checkExtents(ctr);

    	this->checkTree(ctr);
    }

    void replayRemove()
    {
    	this->out()<<"Replay!"<<endl;

    	this->loadAllocator(this->dump_name_);

    	this->checkAllocator(MA_SRC, "Replay: Container Check Failed");

    	Ctr ctr = this->findCtr(ctr_name_);

    	try {
    		testRemovalStep(ctr);
    	}
    	catch (...) {
    		this->commit();
    		this->storeAllocator(this->dump_name_+"-repl");
    		throw;
    	}
    }

    CtrSizeT sampleSize(Int iteration, CtrSizeT size)
    {
    	if (iteration % 3 == 0)
    	{
    		return this->getRandom(size);
    	}
    	else if (iteration % 3 == 1) {
    		return size - 1;
    	}
    	else {
    		return 0;
    	}
    }

    void testRemove() {

    	if (level == -1)
    	{
    		for (Int c = 0; c < Levels; c++)
    		{
    			testRemovalForLevel(c);
    			createAllocator(this->allocator());
    			this->out()<<endl;
    		}
    	}
    	else {
    		testRemovalForLevel(level);
    	}
    }

    void testRemovalForLevel(Int level)
    {
    	this->out()<<"Test for level: "<<level<<endl;

    	Ctr ctr = this->createCtr();
        this->ctr_name_ = ctr.name();

		auto shape = this->sampleTreeShape();

		this->out()<<"shape: "<<shape<<endl;

		RngInputProvider provider(shape, this->getIntTestGenerator());

        this->fillCtr(ctr, provider);

        this->commit();

        try {
            for (Int c = 0; c < this->iterations && ctr.sizes().sum() > 0; c++)
            {
            	this->out()<<"Iteration: "<<c<<endl;

            	if (c == 3) {
            		int a = 0; a++;
            	}

            	auto sizes = ctr.sizes();

            	removal_pos_ = CtrSizesT(-1);

        		removal_pos_[0] = sampleSize(c, sizes[0]);

            	auto iter = ctr.seek(removal_pos_[0]);
            	level_ = 0;

            	for (Int s = 1; s <= level; s++)
            	{
            		auto local_size = iter.substream_size();

            		if (local_size > 0)
            		{
            			removal_pos_[s] = sampleSize(c, local_size);

            			iter.toData(removal_pos_[s]);
            			level_ = s;
            		}
            		else {
            			break;
            		}
            	}

            	auto pos = iter.pos();
            	auto size = iter.size();

            	auto len = size - pos;

            	if (len > 0) {
            		length_ = this->getRandom(len - 1) + 1;
            	}
            	else {
            		length_ = 0;
            	}

            	testRemovalStep(ctr);

                this->out()<<"Sizes: "<<ctr.sizes()<<endl<<endl;

                this->commit();
            }
        }
        catch (...) {
        	this->dump_name_ = this->Store();
            throw;
        }
    }
};

}

#endif