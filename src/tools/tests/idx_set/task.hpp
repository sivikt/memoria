
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_IDX_SET_TASK_HPP_
#define MEMORIA_TESTS_IDX_SET_TASK_HPP_

#include <memoria/memoria.hpp>

#include <memoria/tools/tests.hpp>
#include <memoria/tools/tools.hpp>


#include "params.hpp"

#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>

namespace memoria {

class IdxSetTestTask: public SPTestTask {
public:


private:
	typedef vector<BigInt> PairVector;
	typedef StreamContainerTypesCollection::Factory<SumSet1>::Type IdxSetType;

	PairVector pairs;
	PairVector pairs_sorted;

public:

	IdxSetTestTask(): SPTestTask(new IdxSetTestTaskParams()) {}
	virtual ~IdxSetTestTask() throw() {}

	void CheckIteratorFw(IdxSetType* map, PairVector& pairs)
	{
		map->End().update();
		Int pairs_size = (Int)pairs.size();

		Int idx = 0;
		for (auto iter = map->Begin(); !iter.IsEnd(); )
		{
		    BigInt  key 	= iter.GetKey(0);

		    MEMORIA_TEST_ASSERT1(pairs[idx],   !=, key, idx);

		    iter.Next();
		    idx++;
		}

		MEMORIA_TEST_ASSERT(idx, !=, pairs_size);

		idx = pairs_size - 1;
		for (auto iter = map->RBegin(); !iter.IsStart(); )
		{
			BigInt  key 	= iter.GetKey(0);

			MEMORIA_TEST_ASSERT1(pairs[idx],   !=, key, idx);

			iter.Prev();

			idx--;
		}

		MEMORIA_TEST_ASSERT_EXPR(idx != -1, idx, pairs_size);
	}


	void CheckIteratorBw(IdxSetType* map, PairVector& pairs)
	{
		Int pairs_size = (Int)pairs.size();
		Int idx = pairs_size - 1;

		for (auto iter = map->RBegin(); !iter.IsStart(); )
		{
			BigInt  key 	= iter.GetKey(0);

			MEMORIA_TEST_ASSERT(pairs[idx],   !=, key);

		    iter.Prev();
		    idx--;
		}

		MEMORIA_TEST_ASSERT_EXPR(idx != -1, idx, pairs_size);
	}


	// FIXME: SkipKeyFw/SkipKeyBw are not properly implemented for the IdxSet

	void CheckMultistepForwardIterator(IdxSetType* map)
	{
//		BigInt max = map->GetSize();
//
//		for (Int c = 0; c < 100; c++)
//		{
//			auto iter1 = map->Begin();
//			auto iter2 = iter1;
//
//			cout<<"Iter1.1="<<iter1.prefix(0)<<endl;
//			cout<<"Iter2.1="<<iter2.prefix(0)<<endl;
//
//			BigInt rnd = max > 0 ? GetRandom(max) : 0;
//
//			if (rnd > 0) {
//				iter1.SkipKeyFw(rnd);
//			}
//
//			for (BigInt d = 0; d < rnd; d++)
//			{
//				iter2.NextKey();
//			}
//
//			cout<<"Iter1.2="<<iter1.prefix(0)<<endl;
//			cout<<"Iter2.2="<<iter2.prefix(0)<<endl;
//
//			MEMORIA_TEST_ASSERT_EXPR(iter1 != iter2, iter1.key_idx(), iter2.key_idx());
//		}
	}

	void CheckMultistepBackwardIterator(IdxSetType* map)
	{
//		BigInt max = map->GetSize();
//
//		for (Int c = 0; c < 100; c++)
//		{
//			auto iter1 = map->RBegin();
//			auto iter2 = iter1;
//
//			BigInt rnd = max > 0 ? GetRandom(max) : 0;
//
//			if (rnd > 0) {
//				iter1.SkipKeyBw(rnd);
//			}
//
//			for (BigInt d = 0; d < rnd; d++)
//			{
//				iter2.PrevKey();
//			}
//
//			MEMORIA_TEST_ASSERT_EXPR(iter1 != iter2, iter1.key_idx(), iter2.key_idx());
//		}
	}

	virtual TestStepParams* CreateTestStep(StringRef name) const
	{
		return new IdxSetTestStepParams(name);
	}

	virtual void Replay(ostream& out, TestStepParams* step_params)
	{
		IdxSetTestStepParams* params = static_cast<IdxSetTestStepParams*>(step_params);

		LoadVector(pairs, params->GetPairsDataFile());
		LoadVector(pairs_sorted, params->GetPairsSortedDataFile());

		Allocator allocator;
		LoadAllocator(allocator, params);

		if (params->GetStep() < 3)
		{
			DoTestStep(out, allocator, params);
		}
		else {
			BigInt from = params->GetFrom();
			BigInt to 	= params->GetTo();

			BigInt from_key = pairs_sorted[from];
			BigInt to_key   = pairs_sorted[to] + 1;

			IdxSetType map(allocator, 1);
			map.Remove(from_key, to_key);
		}
	}

	virtual void Run(ostream& out)
	{
		Int SIZE = GetParameters<IdxSetTestTaskParams>()->GetSize();

		for (Int c = 0; c < SIZE; c++)
		{
			pairs.push_back(GetBIRandom());
		}

		IdxSetTestStepParams params;

		params.SetSize(SIZE);

		{
			//Isolate the scope of this allocator
			Allocator allocator;
			IdxSetType map(allocator, 1, true);

			for (Int step = 0; step < 2; step++)
			{
				params.SetStep(step);

				for (Int c = 0; c < SIZE; c++)
				{
					PairVector pairs_sorted_tmp = pairs_sorted;

					try {
						params.SetVectorIdx(c);

						DoTestStep(out, allocator, &params);
					}
					catch (...)
					{
						StorePairs(pairs, pairs_sorted_tmp, params);
						Store(allocator, &params);
						throw;
					}
				}
			}
		}

		Allocator allocator;

		params.SetStep(2);

		for (Int x = 0; x < 4; x++)
		{
			IdxSetType map(allocator, 1, true);
			for (Int c = 0; c < SIZE; c++)
			{
				map.Put(pairs[c], 0);
			}
			allocator.commit();

			pairs_sorted = pairs;
			std::sort(pairs_sorted.begin(), pairs_sorted.end());

			while (map.GetSize() > 0)
			{
				BigInt size = map.GetSize();

				UInt from, to;
				if (x == 0)
				{
					from 	= 0;
					to 		= size - 1;
				}
				else if (x == 1)
				{
					from 	= 0;
					to 		= size/2;
				}
				else if (x == 2)
				{
					from 	= size/2;
					to 		= size - 1;
				}
				else if(size > 3) {
					from 	= GetRandom(size/2 - 1);
					to 		= GetRandom(size/2) + size/2;
				}
				else {
					from 	= 0;
					to 		= size - 1;
				}

				params.SetFrom(from);
				params.SetTo(to);

				BigInt from_key = pairs_sorted[from];
				BigInt to_key   = pairs_sorted[to] + 1;

				PairVector pairs_sorted_tmp = pairs_sorted;
				pairs_sorted.erase(pairs_sorted.begin() + from, pairs_sorted.begin() + to + 1);

				map.Remove(from_key, to_key);

				try {
					MEMORIA_TEST_ASSERT1(pairs_sorted.size(), !=, (UInt)map.GetSize(), x);

					Check(allocator, MEMORIA_SOURCE);
					CheckIteratorFw(&map, pairs_sorted);
					CheckIteratorBw(&map, pairs_sorted);

					allocator.commit();
				}
				catch (...)
				{
					StorePairs(pairs, pairs_sorted_tmp, params);
					Store(allocator, &params);

					throw;
				}
			}
		}
	}

	void StorePairs(const PairVector& pairs, const PairVector& pairs_sorted, IdxSetTestStepParams& params)
	{
		String basic_name = GetTaskName()+ "." + params.GetName();

		String pairs_name = basic_name + ".pairs.txt";
		StoreVector(pairs, pairs_name);
		params.SetPairsDataFile(pairs_name);

		String pairs_sorted_name = basic_name + ".pairs_sorted.txt";
		StoreVector(pairs_sorted, pairs_sorted_name);
		params.SetPairsSortedDataFile(pairs_sorted_name);
	}


	void DoTestStep(ostream& out, Allocator& allocator, const IdxSetTestStepParams* params)
	{
		unique_ptr<IdxSetType> map(new IdxSetType(allocator, 1));

		Int c = params->GetVectorIdx();

		if (params->GetStep() == 0)
		{
			map->Put(pairs[c], 0);

			Check(allocator, MEMORIA_SOURCE);

			AppendToSortedVector(pairs_sorted, pairs[c]);

			CheckIteratorFw(map.get(), pairs_sorted);
			CheckIteratorBw(map.get(), pairs_sorted);
			CheckMultistepForwardIterator(map.get());
			CheckMultistepBackwardIterator(map.get());

			allocator.commit();
		}
		else {
			map->Remove(pairs[c]);

			Check(allocator, MEMORIA_SOURCE);

			BigInt size = params->GetSize() - c - 1;

			MEMORIA_TEST_ASSERT(size, !=, map->GetSize());

			for (UInt x = 0; x < pairs_sorted.size(); x++)
			{
				if (pairs_sorted[x] == pairs[c])
				{
					pairs_sorted.erase(pairs_sorted.begin() + x);
				}
			}

			CheckIteratorFw(map.get(), pairs_sorted);
			CheckIteratorBw(map.get(), pairs_sorted);


			if (c < params->GetSize() - 1)
			{
				CheckMultistepForwardIterator(map.get());
				CheckMultistepBackwardIterator(map.get());
			}

			allocator.commit();
		}
	}
};


}


#endif