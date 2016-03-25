
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

#include <memoria/v1/core/packed/sseq/packed_fse_searchable_seq.hpp>
#include <memoria/v1/core/packed/tree/vle/packed_vle_quick_tree.hpp>
#include <memoria/v1/core/packed/tree/vle/packed_vle_dense_tree.hpp>
#include <memoria/v1/core/packed/tree/fse/packed_fse_quick_tree.hpp>
#include <memoria/v1/core/tools/elias_codec.hpp>

#include <functional>

namespace memoria {
namespace v1 {

using namespace std;

template <
    Int BitsPerSymbol_ = 8,
    typename IndexType = PkdVDTreeT<BigInt, 1<<BitsPerSymbol_, UBigIntI64Codec>,
    template <typename> class ReindexFnType = VLEReindex8Fn,
    template <typename> class SelectFnType  = Seq8SelectFn,
    template <typename> class RankFnType    = Seq8RankFn,
    template <typename> class ToolsFnType   = Seq8ToolsFn
>
struct PackedCxMultiSequenceTypes {
    static const Int BitsPerSymbol = BitsPerSymbol_;

    using Index = IndexType;

    template <typename Seq>
    using ReindexFn = ReindexFnType<Seq>;

    template <typename Seq>
    using SelectFn  = SelectFnType<Seq>;

    template <typename Seq>
    using RankFn    = RankFnType<Seq>;

    template <typename Seq>
    using ToolsFn   = ToolsFnType<Seq>;

};

template <typename Types>
class PackedCxMultiSequence: public PackedAllocator {

    typedef PackedAllocator                                                     Base;
    typedef PackedCxMultiSequence<Types>                                        MyType;

public:

    typedef PkdFQTreeT<Int, 1>                                                  LabelArray;
    typedef typename LabelArray::Values                                         LabelArrayValues;

    typedef PkdFSSeqTypes<
            Types::BitsPerSymbol,
            512,
            typename Types::Index,
            Types::template ReindexFn,
            Types::template SelectFn,
            Types::template RankFn,
            Types::template ToolsFn
    >                                                                           SequenceTypes;
    typedef PkdFSSeq<SequenceTypes>                                             Sequence;

    typedef typename Sequence::SymbolAccessor                                   SymbolAccessor;
    typedef typename Sequence::ConstSymbolAccessor                              ConstSymbolAccessor;

    enum {
        LABELS, SYMBOLS
    };


public:
    PackedCxMultiSequence() {}

    LabelArray* labels() {
        return Base::template get<LabelArray>(LABELS);
    }

    const LabelArray* labels() const {
        return Base::template get<LabelArray>(LABELS);
    }

    Sequence* sequence() {
        return Base::template get<Sequence>(SYMBOLS);
    }

    const Sequence* sequence() const {
        return Base::template get<Sequence>(SYMBOLS);
    }

    static Int empty_size()
    {
        Int labels_block_size   = LabelArray::empty_size();
        Int symbols_block_size  = Sequence::empty_size();

        Int block_size = Base::block_size(labels_block_size + symbols_block_size, 2);

        return block_size;
    }

    void init()
    {
        Int block_size = MyType::empty_size();

        Base::init(block_size, 2);

        Base::template allocateEmpty<LabelArray>(LABELS);
        Base::template allocateEmpty<Sequence>(SYMBOLS);
    }

    Int rank(Int subseq_num, Int to, Int symbol) const
    {
        const Sequence*     seq     = sequence();
        const LabelArray*   labels  = this->labels();

        MEMORIA_V1_ASSERT(to, <=, labels->value(0, subseq_num));

        Int seq_pefix   = labels->sum(0, subseq_num);

        return seq->rank(seq_pefix, seq_pefix + to, symbol);
    }

    SelectResult select(Int subseq_num, Int rank, Int symbol) const
    {
        const Sequence*     seq     = sequence();
        const LabelArray*   labels  = this->labels();

        Int seq_size    = labels->value(0, subseq_num);
        Int seq_prefix  = labels->sum(0, subseq_num);
        Int rank_prefix = seq->rank(seq_prefix, symbol);

        SelectResult result = seq->selectFw(symbol, rank_prefix + rank);
        if (result.idx() - seq_prefix < seq_size)
        {
            return SelectResult(result.idx() - seq_prefix, rank, true);
        }
        else {
            return SelectResult(seq_prefix + seq_size, seq->rank(seq_prefix, seq_prefix + seq_size), false);
        }
    }

    void insertSubsequence(Int idx)
    {
        labels()->insert(idx, LabelArrayValues());
        labels()->reindex();
    }

    void appendSubsequence()
    {
        insertSubsequence(labels()->size());
    }

    void insertSymbol(Int subseq_num, Int idx, Int symbol)
    {
        Sequence* seq       = sequence();
        LabelArray* labels  = this->labels();

        Int seq_prefix  = labels->sum(0, subseq_num);

        MEMORIA_V1_ASSERT(idx, <=, labels->value(0, subseq_num));

        seq->insert(seq_prefix + idx, symbol);

        labels->value(0, subseq_num)++;
//        labels->setValue(0, subseq_num)++;

        labels->reindex();

        seq->reindex();
    }

    void removeSymbol(Int subseq_num, Int idx)
    {
        Sequence* seq       = sequence();
        LabelArray* labels  = this->labels();

        Int seq_prefix  = labels->sum(0, subseq_num);

        MEMORIA_V1_ASSERT(idx, <=, labels->value(0, subseq_num));

        seq->removeSymbol(seq_prefix + idx);

        labels->value(0, subseq_num)--;

        labels->reindex();

        seq->reindex();
    }

    void appendSymbol(Int subseq_num, Int symbol)
    {
        LabelArray* labels  = this->labels();
        Int size            = labels->value(0, subseq_num);

        insertSymbol(subseq_num, size, symbol);
    }

    void remove(Int subseq_num)
    {
        LabelArray* labels  = this->labels();
        MEMORIA_V1_ASSERT(labels->value(0, subseq_num), ==, 0);

        labels->removeSpace(subseq_num, subseq_num + 1);
    }

    Int subseq_size(Int seq_num) const
    {
        return labels()->value(0, seq_num);
    }

    Int length(Int seq_num) const
    {
        return subseq_size(seq_num);
    }

    static Int block_size(Int client_area)
    {
        return Base::block_size(client_area, 2);
    }


    ConstSymbolAccessor
    symbol(Int seq_num, Int idx) const
    {
        Int seq_prefix  = labels()->sum(0, seq_num);
        Int size        = labels()->value(0, seq_num);

        MEMORIA_V1_ASSERT(idx, <, size);

        return sequence()->symbol(seq_prefix + idx);
    }

    SymbolAccessor
    sumbol(Int seq_num, Int idx)
    {
        Int seq_prefix  = labels()->sum(0, seq_num);
        Int size        = labels()->value(seq_num);

        MEMORIA_V1_ASSERT(idx, <, size);

        return sequence()->symbol(seq_prefix + idx);
    }

    void dump(ostream& out = cout, bool multi = true, bool dump_index = true) const
    {
//      if (dump_index) {
//          Base::dump(out);
//      }

        out<<"Sequence Labels: "<<endl;
        labels()->dump(out, dump_index);
        out<<endl;

        if (multi)
        {
            if (dump_index && sequence()->has_index())
            {
                sequence()->index()->dump(out);
            }

            auto values = sequence()->symbols();

            auto labels = this->labels();

            Int offset = 0;

            for (Int c = 0; c <labels->size(); c++)
            {
                Int size = labels->value(0, c);

                out<<"seq: "<<c<<" offset: "<<offset<<" size: "<<size<<endl;

                dumpSymbols<typename Sequence::Value>(out, size, 8, [values, offset](Int idx) {
                    return values[idx + offset];
                });

                offset += size;

                out<<endl<<endl;
            }
        }
        else {
            out<<"Sequence: "<<endl;
            sequence()->dump(out, dump_index);
            out<<endl;
        }
    }
};

}}