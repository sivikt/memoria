
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_BENCHMARKS_vector_map_BATCH_INSERT_HPP_
#define MEMORIA_BENCHMARKS_vector_map_BATCH_INSERT_HPP_

#include "../benchmarks_inc.hpp"

#include <malloc.h>
#include <memory>

namespace memoria {

using namespace std;



class VectorMapBatchInsertBenchmark: public SPBenchmarkTask {

    typedef SPBenchmarkTask Base;

    typedef typename Base::Allocator                                            Allocator;
    typedef typename Base::Profile                                              Profile;

    typedef typename SCtrTF<VectorMap<BigInt, Byte>>::Type                      Ctr;
    typedef typename Ctr::Iterator                                              Iterator;
    typedef typename Ctr::ID                                                    ID;

    typedef typename Ctr::IdxSet                                                SetCtrType;

    typedef typename SetCtrType::Accumulator                                    Accumulator;

    typedef typename SetCtrType::ISubtreeProvider                               ISubtreeProvider;
    typedef typename SetCtrType::DefaultSubtreeProviderBase                     DefaultSubtreeProviderBase;
    typedef typename SetCtrType::NonLeafNodeKeyValuePair                        NonLeafNodeKeyValuePair;
    typedef typename SetCtrType::LeafNodeKeyValuePair                           LeafNodeKeyValuePair;


    class SubtreeProvider: public DefaultSubtreeProviderBase
    {
        typedef DefaultSubtreeProviderBase          Base;

        BigInt data_size_;

    public:
        SubtreeProvider(SetCtrType* ctr, BigInt total, BigInt data_size): Base(*ctr, total), data_size_(data_size)
        {}

        virtual LeafNodeKeyValuePair getLeafKVPair(BigInt begin)
        {
            Accumulator acc;

            acc[0] = 1;
            acc[1] = data_size_;

            return LeafNodeKeyValuePair(acc, SetCtrType::Value());
        }
    };


    Allocator*  allocator_;
    Ctr*        map_;

    BigInt      memory_size;
    Int         data_size_;
public:

    VectorMapBatchInsertBenchmark(StringRef name, Int data_size):
        SPBenchmarkTask(name), memory_size(128*1024*1024), data_size_(data_size)
    {
        Add("memory_size", memory_size);
    }

    virtual ~VectorMapBatchInsertBenchmark() throw() {}

    virtual void Prepare(BenchmarkParameters& params, ostream& out)
    {
        allocator_ = new Allocator();

        map_ = new Ctr(allocator_);

        allocator_->commit();
    }


    virtual void release(ostream& out)
    {
        delete map_;
        delete allocator_;
    }


    virtual void Benchmark(BenchmarkParameters& params, ostream& out)
    {
        Int size = params.x();

        MemBuffer<Byte> data(size * data_size_);

        SubtreeProvider provider(&map_->set(), size, data_size_);

        BigInt total = 0;

        BigInt key_count = 0;

        while (total < memory_size)
        {
            auto i = key_count == 0 ? map_->Begin() : map_->find(getRandom(key_count));

            map_->set().insertSubtree(i.is_iter(), provider);

            i.ba_iter().insert(data);

            total += data.size();
            key_count += size;
        }

        params.operations() = map_->count();
        params.memory()     = map_->size() + map_->count() * 16; //sizeof(BigInt) * 2

        allocator_->commit();
    }
};


}


#endif
