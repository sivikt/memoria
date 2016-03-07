
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_PACKED_SEQUENCE_SELECT_TEST_HPP_
#define MEMORIA_TESTS_PACKED_SEQUENCE_SELECT_TEST_HPP_

#include <memoria/tools/tests.hpp>
#include <memoria/tools/tools.hpp>

#include <memory>
#include <vector>
#include <functional>
#include "pseq_test_base.hpp"

namespace memoria {

using namespace std;

template <
    Int Bits,
    typename IndexType,
    template <typename> class ReindexFnType = BitmapReindexFn,
    template <typename> class SelectFnType  = BitmapSelectFn,
    template <typename> class RankFnType    = BitmapRankFn,
    template <typename> class ToolsFnType   = BitmapToolsFn
>
class PackedSearchableSequenceSelectTest: public PackedSearchableSequenceTestBase<
    Bits,
    IndexType,
    ReindexFnType,
    SelectFnType,
    RankFnType,
    ToolsFnType
> {

    typedef PackedSearchableSequenceSelectTest<
            Bits,
            IndexType,
            ReindexFnType,
            SelectFnType,
            RankFnType,
            ToolsFnType
    >                                                                           MyType;

    typedef PackedSearchableSequenceTestBase<
            Bits,
            IndexType,
            ReindexFnType,
            SelectFnType,
            RankFnType,
            ToolsFnType
    >                                                                           Base;

    using typename Base::Seq;
    using typename Base::SeqPtr;
    using Value = typename Seq::Value;


    static const Int Blocks                 = Seq::Indexes;
    static const Int Symbols                = 1<<Bits;
    static const Int VPB                    = Seq::ValuesPerBranch;

    using Base::getRandom;
    using Base::createEmptySequence;
    using Base::fillRandom;
    using Base::populate;
    using Base::populateRandom;
    using Base::assertIndexCorrect;
    using Base::assertEqual;
    using Base::assertEmpty;
    using Base::out;

public:

    PackedSearchableSequenceSelectTest(StringRef name): Base(name)
    {
        this->size_ = 8192;

        MEMORIA_ADD_TEST(runSelectFromFWTest);
        MEMORIA_ADD_TEST(runSelectFWTest);

        MEMORIA_ADD_TEST(runSelectBWTest);
    }

    virtual ~PackedSearchableSequenceSelectTest() throw() {}



    SelectResult selectFW(const SeqPtr& seq, Int start, Int rank, Value symbol)
    {
        MEMORIA_ASSERT(rank, >, 0);

        Int total = 0;

        for (Int c = start; c < seq->size(); c++)
        {
            total += seq->test(c, symbol);

            if (total == rank)
            {
                return SelectResult(c, rank, true);
            }
        }

        return SelectResult(seq->size(), total, total == rank);
    }


    SelectResult selectBW(const SeqPtr& seq, Int start, Int rank, Value symbol)
    {
        MEMORIA_ASSERT(rank, >, 0);

        Int total = 0;

        for (Int c = start - 1; c >= 0; c--)
        {
            total += seq->test(c, symbol);

            if (total == rank)
            {
                return SelectResult(c, rank, true);
            }
        }

        return SelectResult(-1, total, total == rank);
    }

    void assertSelectFW(const SeqPtr& seq, Int start, Int rank, Value symbol)
    {
        auto result1 = seq->selectFw(start, symbol, rank);
        auto result2 = selectFW(seq, start, rank, symbol);

        AssertEQ(MA_SRC, result1.is_found(),  result2.is_found(), SBuf()<<start<<" "<<rank);

        if (!result1.is_found())
        {
            try {
                AssertEQ(MA_SRC, result1.rank(), result2.rank(), SBuf()<<start<<" "<<rank);
            }
            catch (...) {
                seq->dump(this->out());
                throw;
            }
        }
        else {
            AssertEQ(MA_SRC, result1.idx(),  result2.idx(), SBuf()<<start<<" "<<rank);
        }
    }

    void assertSelectBW(const SeqPtr& seq, Int start, Int rank, Value symbol)
    {
        auto result1 = seq->selectBw(start, symbol, rank);
        auto result2 = selectBW(seq, start, rank, symbol);

        AssertEQ(MA_SRC, result1.is_found(),  result2.is_found(), SBuf()<<start<<" "<<rank);

        if (!result1.is_found())
        {
            AssertEQ(MA_SRC, result1.rank(), result2.rank(), SBuf()<<start<<" "<<rank);
        }
        else {
            AssertEQ(MA_SRC, result1.idx(),  result2.idx(), SBuf()<<start<<" "<<rank);
        }
    }

    vector<Int> createStarts(const SeqPtr& seq)
    {
        Int max_block  = seq->size() / VPB + (seq->size() % VPB == 0 ? 0 : 1);

        vector<Int> starts;

        for (Int block = 0; block < max_block; block++)
        {
            Int block_start = block * VPB;
            Int block_end = block_start + VPB <= (Int)seq->size() ? block_start + VPB : seq->size();

            starts.push_back(block_start);
            starts.push_back(block_start + 1);

            for (Int d = 2; d < (Int)VPB && block_start + d < block_end; d += 128)
            {
                starts.push_back(block_start + d);
            }

            starts.push_back(block_end - 1);
            starts.push_back(block_end);
        }

        return starts;
    }


    vector<Int> createRanks(const SeqPtr& seq, Int start)
    {
        Int max_block  = seq->size() / VPB + (seq->size() % VPB == 0 ? 0 : 1);

        vector<Int> ranks;

        for (Int block = start / VPB; block < max_block; block++)
        {
            Int block_start = block * VPB;
            Int block_end = block_start + VPB <= (Int)seq->size() ? block_start + VPB : seq->size();

            appendRank(ranks, block_start);
            appendRank(ranks, block_start + 1);

            Int rank_delta = 128 / Bits;

            for (Int d = rank_delta; d < VPB; d += rank_delta)
            {
                appendRank(ranks, block_start + d);
            }

            appendRank(ranks, block_end - 1);
            appendRank(ranks, block_end);
        }

        return ranks;
    }

    void appendRank(vector<Int>& v, Int rank)
    {
        if (rank > 0)
        {
            v.push_back(rank);
        }
    }

    struct Pair {
        Int rank;
        Int idx;

        Pair(Int r, Int i): rank(r), idx(i) {}
    };

    vector<Pair> createRanksFW(const SeqPtr& seq, Int start, Value symbol)
    {
        vector<Pair> ranks;

        Int rank = 0;

        for (Int c = start; c < (Int)seq->size(); c++)
        {
            if (seq->test(c, symbol))
            {
                rank++;
                ranks.push_back(Pair(rank, c));
            }
        }

        return ranks;
    }

    vector<Pair> createRanksBW(const SeqPtr& seq, Int start, Value symbol)
    {
        vector<Pair> ranks;

        Int rank = 0;

        for (Int c = start; c >= 0; c--)
        {
            if (seq->test(c, symbol))
            {
                rank++;
                ranks.push_back(Pair(rank, c));
            }
        }

        return ranks;
    }

    void runSelectFromFWTest()
    {
        runSelectFromFWTest(0);
        runSelectFromFWTest(Symbols - 1);
    }

    void runSelectFromFWTest(Value symbol)
    {
        out()<<"Parameters: Bits="<<Bits<<" symbol="<<(Int)symbol<<endl;

        auto seq = createEmptySequence();

        populate(seq, this->size_, symbol);

        vector<Int> starts = createStarts(seq);

        out()<<"Solid bitmap"<<endl;

        for (Int start: starts)
        {
            out()<<start<<endl;

            vector<Int> ranks = createRanks(seq, start);

            for (Int rank: ranks)
            {
                assertSelectFW(seq, start, rank, symbol);
            }
        }

        out()<<endl;
        out()<<"Random bitmap, random positions"<<endl;

        populateRandom(seq, this->size_);
        starts = createStarts(seq);

        for (Int start: starts)
        {
            this->out()<<start<<endl;

            vector<Int> ranks = createRanks(seq, start);

            for (Int rank: ranks)
            {
                assertSelectFW(seq, start, rank, symbol);
            }
        }

        out()<<endl;
        out()<<"Random bitmap, "<<(Int)symbol<<"-set positions"<<endl;

        starts = createStarts(seq);

        for (Int start : starts)
        {
            auto pairs = createRanksFW(seq, start, symbol);

            out()<<start<<endl;

            for (auto pair: pairs)
            {
                auto result = seq->selectFw(start, symbol, pair.rank);

                AssertTrue(MA_SRC, result.is_found());

                if (!result.is_found())
                {
                    AssertEQ(MA_SRC, result.rank(), pair.rank);
                }
                else {
                    AssertEQ(MA_SRC, result.idx(),  pair.idx);
                }
            }
        }

        out()<<endl;
    }


    void runSelectFWTest()
    {
        auto seq = createEmptySequence();

        populateRandom(seq, this->size_);

        Int maxrank_ = seq->rank(1) + 1;

        for (Int rank = 1; rank < maxrank_; rank++)
        {
            auto result1 = seq->selectFw(0, 1, rank);
            auto result2 = seq->selectFw(1, rank);

            auto result3 = selectFW(seq, 0, rank, 1);

            AssertEQ(MA_SRC, result1.is_found(), result2.is_found(), SBuf()<<rank);
            AssertEQ(MA_SRC, result1.is_found(), result3.is_found(), SBuf()<<rank);

            if (result1.is_found())
            {
                AssertEQ(MA_SRC, result1.idx(), result2.idx(), SBuf()<<rank);
                AssertEQ(MA_SRC, result1.idx(), result3.idx(), SBuf()<<rank);
            }
            else {
                AssertEQ(MA_SRC, result1.rank(), result2.rank(), SBuf()<<rank);
                AssertEQ(MA_SRC, result1.rank(), result3.rank(), SBuf()<<rank);
            }
        }
    }


    void runSelectBWTest()
    {
        runSelectBWTest(0);
        runSelectBWTest(Symbols - 1);
    }

    void runSelectBWTest(Value symbol)
    {
        out()<<"Parameters: "<<Bits<<" "<<(Int)symbol<<endl;

        auto seq = createEmptySequence();

        populate(seq, this->size_, symbol);

        vector<Int> starts = createStarts(seq);

        starts.push_back(seq->size());

        out()<<"Solid bitmap"<<endl;

        for (Int start: starts)
        {
            out()<<start<<endl;

            vector<Int> ranks = createRanks(seq, start);

            for (Int rank: ranks)
            {
                assertSelectBW(seq, start, rank, symbol);
            }
        }

        out()<<endl;
        out()<<"Random bitmap, random positions"<<endl;

        populateRandom(seq, this->size_);

        for (Int start: starts)
        {
            this->out()<<start<<endl;

            vector<Int> ranks = createRanks(seq, start);

            for (Int rank: ranks)
            {
                assertSelectBW(seq, start, rank, symbol);
            }
        }

        out()<<endl;
        out()<<"Random bitmap, "<<(Int)symbol<<"-set positions"<<endl;

        for (Int start : starts)
        {
            if (start > 0 && start < seq->size())
            {
                out()<<start<<endl;

                auto pairs = createRanksBW(seq, start-1, symbol);

                for (auto pair: pairs)
                {
                    SelectResult result = seq->selectBw(start, symbol, pair.rank);

                    AssertTrue(MA_SRC, result.is_found(), SBuf()<<start<<" "<<pair.rank);

                    if (!result.is_found())
                    {
                        AssertEQ(MA_SRC, result.rank(), pair.rank, SBuf()<<start<<" "<<pair.rank);
                    }
                    else {
                        AssertEQ(MA_SRC, result.idx(),  pair.idx, SBuf()<<start<<" "<<pair.rank);
                    }
                }
            }
        }

        out()<<endl;

        Int seqrank_ = this->rank(seq, 0, seq->size(), symbol);
        assertSelectBW(seq, seq->size(), seqrank_/2, symbol);
    }
};


}


#endif
