
// Copyright 2016 Victor Smirnov
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

#include <memoria/v1/prototypes/bt/walkers/bt_walker_base.hpp>
#include <memoria/v1/core/tools/assert.hpp>

namespace memoria {
namespace v1 {
namespace bt {


/**********************************************************************/

template <
    typename Types,
    typename MyType
>
class SelectGEForwardWalkerBase: public FindForwardWalkerBase<Types, MyType> {
protected:
    using Base  = FindForwardWalkerBase<Types, MyType>;
    using CtrSizeT   = typename Types::CtrSizeT;

public:

    SelectGEForwardWalkerBase(Int symbol, CtrSizeT rank):
        Base(symbol, rank, SearchType::GE)
    {}

    template <Int StreamIdx, typename Tree>
    StreamOpResult find_non_leaf(const Tree* tree, bool root, Int index, Int start)
    {
        auto size = tree->size();

        if (start < size)
        {
            auto k = Base::target_ - Base::sum_;

            auto result = tree->findForward(Base::search_type_, index, start, k);

            Int nearest_nz_idx = tree->findNZLT(index, start);

            if (nearest_nz_idx >= result.idx())
            {
            	Base::sum_ += result.prefix();
            	return StreamOpResult(result.idx(), start, result.idx() >= size, false);
            }
            else {
            	Base::sum_ += tree->sum(index, start, nearest_nz_idx);
            	return StreamOpResult(nearest_nz_idx, start, result.idx() >= size, false);
            }
        }
        else {
            return StreamOpResult(size, start, true, true);
        }
    }


    template <Int StreamIdx, typename Seq>
    StreamOpResult find_leaf(const Seq* seq, Int start)
    {
        MEMORIA_V1_ASSERT_TRUE(seq);

        auto& sum       = Base::sum_;
        auto symbol     = Base::leaf_index();

        CtrSizeT rank   = Base::target_ - sum;
        auto result     = self().template select<StreamIdx>(seq, start, symbol, rank);

        if (result.is_found())
        {
            sum  += rank;
            return StreamOpResult(result.idx(), start, false);
        }
        else {
            Int size = seq->size();

            sum  += result.rank();
            return StreamOpResult(size, start, true);
        }
    }

    MyType& self() {return *T2T<MyType*>(this);}
    const MyType& self() const {return *T2T<const MyType*>(this);}
};




template <
    typename Types
>
class SelectGEForwardWalker: public SelectGEForwardWalkerBase<Types,SelectGEForwardWalker<Types>> {

    using Base = SelectGEForwardWalkerBase<Types, SelectGEForwardWalker<Types>>;
    using typename Base::CtrSizeT;

protected:
    using Base::direction_;

public:
    SelectGEForwardWalker(Int symbol, CtrSizeT rank):
        Base(symbol, rank)
    {}

    template <Int StreamIdx, typename Seq>
    SelectResult select(const Seq* seq, Int start, Int symbol, CtrSizeT rank)
    {
        if (direction_ == WalkDirection::DOWN)
      {
          MEMORIA_V1_ASSERT(start, ==, 0);
          return seq->selectGEFW(rank, symbol);
      }
      else {
          return seq->selectGEFW(start, rank, symbol);
      }
    }
};




/*
template <
    typename Types,
    typename MyType
>
class SelectBackwardWalkerBase: public FindBackwardWalkerBase<Types, MyType> {
protected:
    using Base  = FindBackwardWalkerBase<Types, MyType>;
    using CtrSizeT   = typename Types::CtrSizeT;

public:

    SelectBackwardWalkerBase(Int symbol, CtrSizeT rank):
        Base(symbol, rank, SearchType::GE)
    {}


    template <Int StreamIdx, typename Seq>
    StreamOpResult find_leaf(const Seq* seq, Int start)
    {
        MEMORIA_V1_ASSERT_TRUE(seq);

        auto size = seq->size();

        if (start > size) start = size;

        CtrSizeT target = Base::target_ - Base::sum_;

        auto& sum       = Base::sum_;
        auto symbol     = Base::leaf_index();
        auto result     = self().template select<StreamIdx>(seq, start, symbol, target);

        if (result.is_found())
        {
            sum += target;
            return StreamOpResult(result.idx(), start, false);
        }
        else {
            sum += result.rank();
            return StreamOpResult(-1, start, true);
        }
    }

    MyType& self() {return *T2T<MyType*>(this);}
    const MyType& self() const {return *T2T<const MyType*>(this);}
};




template <
    typename Types
>
class SelectBackwardWalker: public SelectBackwardWalkerBase<Types, SelectBackwardWalker<Types>> {

    using Base  = SelectBackwardWalkerBase<Types, SelectBackwardWalker<Types>>;
    using CtrSizeT   = typename Base::CtrSizeT;

public:

    SelectBackwardWalker(Int symbol, CtrSizeT target):
        Base(symbol, target)
    {}

    template <Int StreamIdx, typename Seq>
    SelectResult select(const Seq* seq, Int start, Int symbol, CtrSizeT rank)
    {
        return seq->selectBW(start, rank, symbol);
    }
};
*/



}
}}
