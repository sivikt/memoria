
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

#include <memoria/prototypes/bt/bt_walkers.hpp>

#include <memoria/core/packed/sseq/packed_fse_searchable_seq.hpp>

namespace memoria {
namespace louds {


template <typename Types>
class RankFWWalker: public bt::FindForwardWalkerBase<Types, RankFWWalker<Types>> {
    typedef bt::FindForwardWalkerBase<Types, RankFWWalker<Types>>               Base;
    typedef typename Base::Key                                                  Key;

    int64_t rank_ = 0;

    int32_t symbol_;

public:
    typedef typename Base::Iterator                                             Iterator;
    typedef typename Base::ResultType                                           ResultType;

    RankFWWalker(int32_t stream, int32_t index, Key target): Base(stream, 0, target)
    {
        Base::search_type_  = SearchType::GT;
        symbol_             = index;
    }

    int64_t rank() const
    {
        return rank_;
    }

    template <int32_t Idx, typename Tree>
    ResultType stream(Tree&& tree, int32_t start)
    {
        return Base::template stream<Idx>(tree, start);
    }

    template <int32_t StreamIdx, typename StreamType, typename Result>
    void postProcessStream(StreamType&& stream, int32_t start, const Result& result)
    {
        int32_t size = stream->size();

        if (result.local_pos() < size)
        {
            rank_ += stream->sum(symbol_ + 1, start, result.local_pos());
        }
        else
        {
            rank_ += stream->sum(symbol_ + 1, start, size);
        }
    }

    template <int32_t Idx, typename StreamTypes>
    ResultType stream(const PkdFSSeq<StreamTypes>* seq, int32_t start)
    {
        MEMORIA_V1_ASSERT_TRUE(seq != nullptr);

        auto& sum       = Base::sum_;

        int64_t offset   = Base::target_ - sum;

        int32_t size        = seq->size();

        if (start + offset < size)
        {
            rank_ += seq->rank(start, start + offset, symbol_);

            sum += offset;

            return start + offset;
        }
        else {
            rank_ += seq->rank(start, seq->size(), symbol_);

            sum += (size - start);

            return size;
        }
    }

    int64_t finish(Iterator& iter, int32_t idx)
    {
        iter.local_pos() = idx;

        int64_t rank1 = symbol_ ? rank_ : this->sum_ - rank_;

        if (idx >= 0)
        {
            iter.iter_cache().add(this->sum_, rank1);
        }
        else {
            iter.iter_cache().setup(-1, 0);
        }

        iter.iter_cache().setRank1(iter.rank(1));

        return rank_;
    }
};




template <typename Types>
class RankBWWalker: public bt::FindBackwardWalkerBase<Types, RankBWWalker<Types>> {
    typedef bt::FindBackwardWalkerBase<Types, RankBWWalker<Types>>      Base;
    typedef typename Base::Key                                                          Key;

    int64_t rank_ = 0;

    int32_t symbol_;

public:
    typedef typename Base::Iterator                                             Iterator;
    typedef typename Base::ResultType                                           ResultType;

    RankBWWalker(int32_t stream, int32_t index, Key target): Base(stream, 0, target)
    {
        Base::search_type_  = SearchType::GT;
        symbol_             = index;
    }

    int64_t rank() const
    {
        return rank_;
    }

    template <int32_t Idx, typename Tree>
    ResultType stream(Tree&& tree, int32_t start)
    {
        return Base::template stream<Idx>(tree, start);
    }

    template <int32_t StreamIdx, typename StreamType, typename Result>
    void postProcessStream(StreamType&& stream, int32_t start, const Result& result)
    {
        if (result.local_pos() >= 0)
        {
            rank_ += stream->sum(symbol_ + 1, result.local_pos() + 1, start + 1);
        }
        else {
            rank_ += stream->sum(symbol_ + 1, 0, start + 1);
        }
    }

    template <int32_t Idx, typename StreamTypes>
    ResultType stream(const PkdFSSeq<StreamTypes>* seq, int32_t start)
    {
        MEMORIA_V1_ASSERT_TRUE(seq != nullptr);

        auto& sum       = Base::sum_;
        int64_t offset   = Base::target_ - sum;

        if (start - offset >= 0)
        {
            rank_ += seq->rank(start - offset, start, symbol_);

            sum += offset;
            return start - offset;
        }
        else {
            rank_ += seq->rank(0, start, symbol_);

            sum += start;
            return -1;
        }
    }

    int64_t finish(Iterator& iter, int32_t idx)
    {
        iter.local_pos() = idx;

        int64_t rank1 = symbol_ ? rank_ : this->sum_ - rank_;

        if (idx >= 0)
        {
            iter.iter_cache().sub(this->sum_, rank1);
        }
        else {
            iter.iter_cache().sub(iter.iter_cache().pos() + 1, rank1);
        }

        iter.iter_cache().setRank1(iter.rank(1));

        return rank_;
    }
};

}}