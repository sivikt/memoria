
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

#include <memoria/v1/core/tools/bitmap_select.hpp>


namespace memoria {
namespace v1 {


template <typename Seq>
class BitmapSelectFn {

    static const Int BitsPerSymbol                                              = Seq::BitsPerSymbol;

    static_assert(BitsPerSymbol == 1, "BitmapSelectFn<> can only be used with 1-bit sequences");

    const Seq& seq_;
public:
    BitmapSelectFn(const Seq& seq): seq_(seq) {}

    SelectResult operator()(Int start, Int end, Int symbol, BigInt rank)
    {
        if (symbol)
        {
            return Select1FW(seq_.symbols(), start, end, rank);
        }
        else {
            return Select0FW(seq_.symbols(), start, end, rank);
        }
    }
};


template <typename Seq>
class SeqSelectFn {
    typedef typename Seq::Value                                                 Value;

    static const Int BitsPerSymbol                                              = Seq::BitsPerSymbol;

    const Seq& seq_;

    static_assert(BitsPerSymbol > 1 && BitsPerSymbol < 8,
            "SeqSelectFn<> can only be used with 2-7-bit sequences");

public:
    SeqSelectFn(const Seq& seq): seq_(seq) {}

    SelectResult operator()(Int start, Int end, Value symbol, Int rank)
    {
        Int cnt = 0;

        auto buf = seq_.symbols();

        for (Int c = start * BitsPerSymbol; c < end * BitsPerSymbol; c += BitsPerSymbol)
        {
            if (TestBits(buf, c, symbol, BitsPerSymbol))
            {
                cnt++;

                if (cnt == rank)
                {
                    return SelectResult(c / BitsPerSymbol, rank, true);
                }
            }
        }

        return SelectResult(end, cnt, false);
    }
};







template <typename Seq>
class Seq8SelectFn {

    static const Int BitsPerSymbol                                              = Seq::BitsPerSymbol;

    static_assert(BitsPerSymbol == 8,
            "Seq8SelectFn<> can only be used with 8-bit sequences");

    const Seq& seq_;

public:
    Seq8SelectFn(const Seq& seq): seq_(seq) {}

    SelectResult operator()(Int start, Int end, Int symbol, Int rank)
    {
        Int cnt = 0;

        auto buf = seq_.symbols();

        for (Int c = start; c < end; c++)
        {
            if (buf[c] == symbol)
            {
                cnt++;
                if (cnt == rank)
                {
                    return SelectResult(c, rank, true);
                }
            }
        }

        return SelectResult(end, cnt, false);
    }
};


}}