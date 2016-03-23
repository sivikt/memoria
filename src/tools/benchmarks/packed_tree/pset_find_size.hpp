
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include "../benchmarks_inc.hpp"

#include <memoria/v1/core/packed/tree/packed_fse_tree.hpp>

#include <malloc.h>
#include <memory>

namespace memoria {
namespace v1 {

using namespace std;



template <Int BranchingFactor_>
class PSetSizeBenchmark: public BenchmarkTask {

    static const Int Blocks                                                     = 1;

    typedef Packed2TreeTypes<
            BigInt,
            BigInt,
            Blocks,
            ValueFSECodec,
            BranchingFactor_,
            BranchingFactor_
    >                                                                           Types;

    typedef PkdFTree<Types>                                                     Map;

    typedef typename Types::Value                                               Key;
    typedef typename Map::Values                                                Values;


    PackedAllocator*    allocator_;
    Map*                map_;
    Int*                rd_array_;


public:

    PSetSizeBenchmark():
        BenchmarkTask("FindSize."+toString(BranchingFactor_))
    {
        average = 10;
    }

    virtual ~PSetSizeBenchmark() throw() {}

    virtual void Prepare(BenchmarkParameters& params, ostream& out)
    {
        Int tree_size       = params.x();
        Int tree_block_size = Map::block_size(tree_size);
        Int block_size      = PackedAllocator::block_size(tree_block_size, 1);

        void* block         = malloc(block_size);

        allocator_ = T2T<PackedAllocator*>(block);
        allocator_->init(block_size, 1);
        allocator_->setTopLevelAllocator();

        map_ = allocator_->template allocate<Map>(0, allocator_->client_area());

        Values one = {1};

        map_->insert(0, map_->max_size(), [&](){return one;});
        map_->reindex();

        rd_array_ = new Int[params.operations()];
        for (Int c = 0; c < params.operations(); c++)
        {
            rd_array_[c] = getRandom(map_->size());
        }
    }

    virtual void release(ostream& out)
    {
        free(allocator_);
        delete[] rd_array_;
    }

    virtual void Benchmark(BenchmarkParameters& params, ostream& out)
    {
        for (Int c = 0; c < params.operations(); c++)
        {
            BigInt key = rd_array_[c];
            auto result = map_->findGEForward(0, 0, key);
            if (key && result.idx() != key - 1)
            {
                // this shouldn't happen
                cout<<"MISS! "<<key<<endl;
                out<<"MISS! "<<key<<endl;
            }
        }
    }
};


}}