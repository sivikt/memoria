
// Copyright 2012 Victor Smirnov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


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