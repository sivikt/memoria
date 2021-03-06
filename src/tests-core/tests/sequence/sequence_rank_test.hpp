
// Copyright 2013 Victor Smirnov
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

#include "sequence_test_base.hpp"

#include <vector>

namespace memoria {
namespace tests {

template <int32_t BitsPerSymbol, bool Dense = true>
class SequenceRankTest: public SequenceTestBase<BitsPerSymbol, Dense> {

    using MyType = SequenceRankTest<BitsPerSymbol, Dense>;
    using Base   = SequenceTestBase<BitsPerSymbol, Dense>;


    using typename Base::Iterator;
    using typename Base::Ctr;
    using typename Base::CtrName;


    int32_t iterations_ = 100000;

    using Base::commit;
    using Base::drop;
    using Base::branch;
    using Base::allocator;
    using Base::snapshot;
    using Base::check;
    using Base::out;
    using Base::fillRandomSeq;
    using Base::size_;
    using Base::rank;
    using Base::storeAllocator;
    using Base::isReplayMode;
    using Base::getResourcePath;
    using Base::ctr_name_;
    using Base::getRandom;



public:
    SequenceRankTest()
    {
        Base::size_ = 300000;
    }

    MMA_STATE_FILEDS(iterations_)

    static void init_suite(TestSuite& suite)
    {
        MMA_CLASS_TESTS(suite, testIterRank, testCtrRank);
    }

    void testCtrRank()
    {
        auto snp = branch();

        auto ctr = create<CtrName>(snp);

        auto seq = fillRandomSeq(*ctr.get(), size_);

        check(MA_SRC);

        for (int32_t c = 0; c < iterations_; c++)
        {
            out() << c << std::endl;

            int32_t pos     = getRandom(size_);
            int32_t symbol  = getRandom(Base::Symbols);

            int64_t rank1 = ctr->rank(pos, symbol);
            int64_t rank2 = seq->rank(pos, symbol);

            assert_equals(rank1, rank2);
        }

        commit();
    }


    void testIterRank()
    {
        auto snp = branch();

        auto ctr = create<CtrName>(snp);

        auto seq = fillRandomSeq(*ctr.get(), size_);

        for (int32_t c = 0; c < iterations_; c++)
        {
            out() << c << std::endl;

            int32_t pos1    = getRandom(size_);
            int32_t pos2    = pos1 + getRandom(size_ - pos1);
            int32_t symbol  = getRandom(Base::Symbols);

            auto iter   = ctr->seek(pos1);

            int64_t rank1 = iter->rankFw(pos2 - pos1, symbol);
            int64_t rank2 = seq->rank(pos1, pos2, symbol);

            assert_equals(rank1, rank2);
            assert_equals(iter->pos(), pos2);

            int64_t rank3 = iter->rank(pos1 - pos2, symbol);

            assert_equals(iter->pos(), pos1);
            assert_equals(rank1, rank3);
        }

        commit();
    }
};

}}
