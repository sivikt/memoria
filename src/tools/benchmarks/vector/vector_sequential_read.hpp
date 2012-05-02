
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_BENCHMARKS_VECTOR_VECTOR_SEQUENTIAL_READ_HPP_
#define MEMORIA_BENCHMARKS_VECTOR_VECTOR_SEQUENTIAL_READ_HPP_

#include "../benchmarks_inc.hpp"

#include <malloc.h>
#include <memory>

namespace memoria {

using namespace std;



class VectorSequentialReadBenchmark: public SPBenchmarkTask {


	typedef SPBenchmarkTask Base;

	typedef typename Base::Allocator 	Allocator;
	typedef typename Base::Profile 		Profile;

	typedef typename SmallCtrTypeFactory::Factory<Root>::Type 		RootCtr;
	typedef typename SmallCtrTypeFactory::Factory<Vector>::Type 	VectorCtr;
	typedef typename VectorCtr::Iterator							Iterator;
	typedef typename VectorCtr::ID									ID;
	typedef typename VectorCtr::Accumulator							Accumulator;


	typedef typename VectorCtr::Key									Key;



	Allocator* allocator_;
	VectorCtr* ctr_;

	Int result_;

public:

	VectorSequentialReadBenchmark():
		SPBenchmarkTask("SequentialRead")
	{
		RootCtr::Init();
		VectorCtr::Init();
	}

	virtual ~VectorSequentialReadBenchmark() throw() {}

	virtual void Prepare(BenchmarkParameters& params, ostream& out)
	{
		allocator_ = new Allocator();

		Int size = params.x();

		ctr_ = new VectorCtr(*allocator_);

		Iterator i = ctr_->Seek(0);

		Byte array[1024];

		for (Int c = 0; c < size/(Int)sizeof(array); c++)
		{
			for (Int d = 0; d < (Int)sizeof(array); d++)
			{
				array[d] = GetRandom(256);
			}

			i.Insert(ArrayData(sizeof(array), array));
		}
	}

	virtual void Release(ostream& out)
	{
		delete ctr_;
		delete allocator_;
	}

	virtual void Benchmark(BenchmarkParameters& params, ostream& out)
	{
		Byte array[4096];
		BigInt total = 0;

		for (Int c = 0; c < params.operations();)
		{
			int a = 0; a++;
			for (Iterator i = ctr_->Seek(0); !i.IsEof() && c < params.operations(); c++)
			{
				ArrayData data(GetRandom(256), array);
				total += i.Read(data);
			}
		}
	}

	virtual String GetGraphName()
	{
		return "Memoria Vector<BigInt> Sequential Read";
	}
};


}


#endif
