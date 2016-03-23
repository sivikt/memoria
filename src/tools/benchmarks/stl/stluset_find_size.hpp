
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../benchmarks_inc.hpp"

#include "custom_allocator.hpp"

#include <malloc.h>
#include <memory>
#include <unordered_set>

namespace memoria {
namespace v1 {

using namespace std;




class StlUsetSizeBenchmark: public BenchmarkTask {


    typedef BigInt                          Key;
    typedef unordered_set<
                Key,
                hash<Key>,
                std::equal_to<Key>,
                CustomAllocator<Key>
    >                                       Map;

    Map*            map_;
    Int             result_;
    Int*            rd_array_;

public:

    StlUsetSizeBenchmark(StringRef name): BenchmarkTask(name) {}

    virtual ~StlUsetSizeBenchmark() throw() {}

    virtual void Prepare(BenchmarkParameters& params, ostream& out)
    {
        map_ = new Map();

        AllocatorBase<>::reset();

        for (Int c = 0; c < params.x(); c++)
        {
            map_->insert(c);
        }

        rd_array_ = new Int[params.operations()];
        for (Int c = 0; c < params.operations(); c++)
        {
            rd_array_[c] = getRandom(map_->size());
        }
    }

    virtual void release(ostream& out)
    {
        delete map_;
        delete[] rd_array_;
    }

    virtual void Benchmark(BenchmarkParameters& params, ostream& out)
    {
        for (Int c = 0; c < params.operations(); c++)
        {
            result_ = (map_->find(rd_array_[c]) != map_->end());
        }
    }
};


}}