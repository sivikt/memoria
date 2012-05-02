
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_BENCHMARKS_VECTOR_MAP_VECTOR_MAP_RANDOM_READ_HPP_
#define MEMORIA_BENCHMARKS_VECTOR_MAP_VECTOR_MAP_RANDOM_READ_HPP_

#include "../benchmarks_inc.hpp"

#include <malloc.h>
#include <memory>

namespace memoria {

using namespace std;




class VectorMapRandomReadBenchmark: public SPBenchmarkTask {

	typedef SPBenchmarkTask Base;

	typedef typename Base::Allocator 	Allocator;
	typedef typename Base::Profile 		Profile;

	typedef typename SmallCtrTypeFactory::Factory<Root>::Type 		RootCtr;
	typedef typename SmallCtrTypeFactory::Factory<VectorMap>::Type 	Ctr;
	typedef typename Ctr::Iterator									Iterator;

	static const Int MAX_DATA_SIZE									= 256;

	Allocator* allocator_;
	Ctr* ctr_;

	Int result_;

	Int* rd_array_;

public:

	VectorMapRandomReadBenchmark():
		SPBenchmarkTask("RandomRead")
	{
		RootCtr::Init();
		Ctr::Init();
	}

	virtual ~VectorMapRandomReadBenchmark() throw() {}

	virtual void Prepare(BenchmarkParameters& params, ostream& out)
	{
		allocator_ = new Allocator();

		Int size = params.x();

		String resource_name = "VectorMap."+ToString(size)+".dump";

		if (IsResourceExists(resource_name))
		{
			LoadResource(*allocator_, resource_name);

			ctr_ = new Ctr(*allocator_, 1);
		}
		else {
			ctr_ = new Ctr(*allocator_, 1, true);

			Byte array[MAX_DATA_SIZE];

			for (Int c = 0; c < size; c++)
			{
				for (Int d = 0; d < 128; d++)
				{
					array[d] = GetRandom(256);
				}

				Iterator i = ctr_->Create();

				i = ArrayData(GetRandom(sizeof(array)), array);
			}

			allocator_->commit();
			StoreResource(*allocator_, resource_name);
		}

		rd_array_ = new Int[params.operations()];
		for (Int c = 0; c < params.operations(); c++)
		{
			rd_array_[c] = GetRandom(size - 1) + 1;
		}
	}

	virtual void Release(ostream& out)
	{
		delete ctr_;
		delete allocator_;
		delete[] rd_array_;
	}

	virtual void Benchmark(BenchmarkParameters& params, ostream& out)
	{
		Byte array[MAX_DATA_SIZE];
		ArrayData data(sizeof(array), array);

		for (Int c = 0; c < params.operations(); c++)
		{
			ctr_->Find(rd_array_[c]).Read(data);
		}
	}

	virtual String GetGraphName()
	{
		return "Memoria Vector<BigInt>";
	}
};


}


#endif
