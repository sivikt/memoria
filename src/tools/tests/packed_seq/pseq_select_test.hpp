
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_PACKED_SEQ_PSEQ_SELECT_TEST_HPP_
#define MEMORIA_TESTS_PACKED_SEQ_PSEQ_SELECT_TEST_HPP_

#include <memoria/tools/tests.hpp>
#include <memoria/tools/tools.hpp>

#include <memoria/prototypes/btree/tools.hpp>

#include <memoria/core/pmap/packed_seq.hpp>

#include <memory>
#include <vector>
#include <functional>

namespace memoria {

using namespace std;

template <Int Bits>
class PSeqSelectTest: public TestTask {

    typedef PSeqSelectTest MyType;

    typedef PackedSeqTypes<
    		UInt,
    		UBigInt,
    		Bits,
    		PackedSeqBranchingFactor,
    		PackedSeqValuesPerBranch / (Bits * 2)
    > 																			Types;


    typedef typename Types::IndexKey        Key;
    typedef typename Types::Value           Value;

    typedef PackedSeq<Types>            	Seq;

    static const Int Blocks                 = Seq::Blocks;
    static const Int Symbols                = 1<<Seq::Bits;
    static const Int VPB					= Seq::ValuesPerBranch;


    typedef function<void (MyType*, const Seq*, size_t, size_t)> 				AssertSelectFn;


public:

    PSeqSelectTest(): TestTask((SBuf()<<"Select."<<Bits).str())
    {
        MEMORIA_ADD_TEST(runSelect1FWTest);
        MEMORIA_ADD_TEST(runSelect0FWTest);
    }

    virtual ~PSeqSelectTest() throw() {}

    void populate(Seq* seq, Int size, Value value = 0)
    {
    	for (Int c = 0; c < size; c++)
    	{
    		seq->value(c) = value;
    	}

    	for (Int c = size; c < seq->maxSize(); c++)
    	{
    		seq->value(c) = 0;
    	}

    	seq->size() = size;
    	seq->reindex();
    }

    void populateRandom(Seq* seq, Int size)
    {
    	for (Int c = 0; c < size; c++)
    	{
    		seq->value(c) = getBIRandom(Symbols);
    	}

    	for (Int c = size; c < seq->maxSize(); c++)
    	{
    		seq->value(c) = 0;
    	}

    	seq->size() = size;
    	seq->reindex();
    }

    SelectResult select1FW(const Seq* seq, size_t start, size_t rank)
    {
    	const Value* bitmap = seq->valuesBlock();

    	size_t total = 0;

    	for (size_t c = start; c < (size_t)seq->maxSize(); c++)
    	{
    		if (total == rank)
    		{
    			return SelectResult(c, rank, true);
    		}

    		total += GetBit(bitmap, c);
    	}

    	return SelectResult(seq->maxSize(), total, total == rank);
    }

    SelectResult select0FW(const Seq* seq, size_t start, size_t rank)
    {
    	const Value* bitmap = seq->valuesBlock();

    	size_t total = 0;

    	for (size_t c = start; c < (size_t)seq->maxSize(); c++)
    	{
    		if (total == rank)
    		{
    			return SelectResult(c, rank, true);
    		}

    		total += 1 - GetBit(bitmap, c);
    	}

    	return SelectResult(seq->maxSize(), total, total == rank);
    }

    void assertSelect1FW(const Seq* seq, size_t start, size_t rank)
    {
    	if (start == 1538 && rank == 6400)
    	{
    		int a = 0; a++;
    	}

    	auto result1 = seq->selectFW(start, 1, rank);
    	auto result2 = select1FW(seq, start, rank);

    	AssertEQ(MA_SRC, result1.is_found(),  result2.is_found(), SBuf()<<start<<" "<<rank);
    	AssertEQ(MA_SRC, result1.idx(),  result2.idx(), SBuf()<<start<<" "<<rank);
    	AssertEQ(MA_SRC, result1.rank(), result2.rank(), SBuf()<<start<<" "<<rank);
    }

    void assertSelect0FW(const Seq* seq, size_t start, size_t rank)
    {


    	auto result1 = seq->selectFW(start, 0, rank);
    	auto result2 = select0FW(seq, start, rank);

    	AssertEQ(MA_SRC, result1.is_found(),  result2.is_found(), SBuf()<<start<<" "<<rank);
    	AssertEQ(MA_SRC, result1.idx(),  result2.idx(), SBuf()<<start<<" "<<rank);
    	AssertEQ(MA_SRC, result1.rank(), result2.rank(), SBuf()<<start<<" "<<rank);
    }


    vector<size_t> createStarts(const Seq* seq)
	{
    	size_t max_block  = seq->size() / VPB + (seq->size() % VPB == 0 ? 0 : 1);

    	vector<size_t> starts;

    	for (size_t block = 0; block < max_block; block++)
    	{
    		size_t block_start = block * VPB;
    		size_t block_end = block_start + VPB <= (size_t)seq->size() ? block_start + VPB : seq->size();

    		starts.push_back(block_start);
    		starts.push_back(block_start + 1);

    		for (size_t d = 2; d < (size_t)VPB && block_start + d < block_end; d += 128)
    		{
    			starts.push_back(block_start + d);
    		}

    		starts.push_back(block_end - 1);
    		starts.push_back(block_end);
    	}

    	return starts;
	}


    vector<size_t> createFWRanks(const Seq* seq, size_t start)
    {
    	size_t max_block  = seq->size() / VPB + (seq->size() % VPB == 0 ? 0 : 1);

    	vector<size_t> starts;

    	for (size_t block = start / VPB; block < max_block; block++)
    	{
    		size_t block_start = block * VPB;
    		size_t block_end = block_start + VPB <= (size_t)seq->size() ? block_start + VPB : seq->size();

    		starts.push_back(block_start);
    		starts.push_back(block_start + 1);

    		for (size_t d = 128; d < (size_t)VPB; d += 128)
    		{
    			starts.push_back(block_start + d);
    		}

    		starts.push_back(block_end - 1);
    		starts.push_back(block_end);
    	}

    	return starts;
    }

    Seq* createEmptySequence() const
    {
    	Int buffer_size     = Bits < 8 ? 2048*Bits : 8192*Bits;

    	Byte* buffer       	= new Byte[buffer_size];

    	memset(buffer, 0, buffer_size);

    	Seq* seq = T2T<Seq*>(buffer);
    	seq->initByBlock(buffer_size - sizeof(Seq));

    	return seq;
    }

    void runSelect1FWTest(ostream& out)
    {
    	runSelectFWTest(out, &MyType::assertSelect1FW, 1);
    }

    void runSelect0FWTest(ostream& out)
    {
    	runSelectFWTest(out, &MyType::assertSelect0FW, 0);
    }

    void runSelectFWTest(ostream& out, AssertSelectFn assert_fn, Value value)
    {
    	out<<"runSelectFWTest: "<<Bits<<" "<<value<<endl;

    	Seq* seq = createEmptySequence();

    	populate(seq, seq->maxSize(), value);

    	vector<size_t> starts = createStarts(seq);

    	for (size_t start: starts)
    	{
    		cout<<start<<endl;

    		vector<size_t> ranks = createFWRanks(seq, start);

    		for (size_t rank: ranks)
    		{
    			assert_fn(this, seq, start, rank);
    		}
    	}

    	populateRandom(seq, seq->maxSize());

    	for (size_t start: starts)
    	{
    		cout<<start<<endl;

    		vector<size_t> ranks = createFWRanks(seq, start);

    		for (size_t rank: ranks)
    		{
    			assert_fn(this, seq, start, rank);
    		}
    	}

    }
};


}


#endif
