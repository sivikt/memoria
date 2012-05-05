
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_BENCHMARKS_VECTOR_STL_VECTOR_READ_HPP_
#define MEMORIA_BENCHMARKS_VECTOR_STL_VECTOR_READ_HPP_

#include "../benchmarks_inc.hpp"

#include <malloc.h>
#include <memory>

namespace memoria {

using namespace std;



class StlVectorReadBenchmark: public BenchmarkTask {

	typedef BenchmarkTask Base;

	typedef std::vector<BigInt> VectorCtr;

	VectorCtr* ctr_;

	Int result_;

	Int* rd_array_;

public:

	StlVectorReadBenchmark(StringRef graph_name = "std::vector<BigInt> Read"):
		BenchmarkTask("StlVectorRead", graph_name)
	{
	}

	virtual ~StlVectorReadBenchmark() throw() {}

	virtual void Prepare(BenchmarkParameters& params, ostream& out)
	{
		Int size = params.x();

		ctr_ = new VectorCtr();

		for (Int c = 0; c < size; c++)
		{
			BigInt value = GetRandom(10000);
			ctr_->push_back(value);
		}

		rd_array_ = new Int[params.operations()];
		for (Int c = 0; c < params.operations(); c++)
		{
			rd_array_[c] = GetRandom(size);
		}
	}

	virtual void Release(ostream& out)
	{
		delete ctr_;
		delete[] rd_array_;
	}

	virtual void Benchmark(BenchmarkParameters& params, ostream& out)
	{
		for (Int c = 0; c < params.operations(); c++)
		{
			BigInt value = ctr_->operator[](rd_array_[c]);
			result_ += value;
		}
	}
};


}


#endif
