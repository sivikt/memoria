
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


#include <memoria/v1/core/tools/isequencedata.hpp>
#include <memoria/v1/core/packed/sseq/packed_fse_searchable_seq.hpp>

#include <malloc.h>
#include <iostream>
#include <functional>

namespace memoria {
namespace v1 {

template <typename Seq>
class SequenceDataSourceAdapter: public ISequenceDataSource<typename Seq::Value, Seq::BitsPerSymbol> {

    static const SizeT Bits     = Seq::BitsPerSymbol;

    typedef typename Seq::Value T;

    SizeT   start0_;
    SizeT   start_;
    SizeT   length_;

    const Seq*  sequence_;
public:

    SequenceDataSourceAdapter(const Seq* sequence, SizeT start, SizeT length):
        start0_(start),
        start_(start),
        length_(length),
        sequence_(sequence)
    {}


    virtual SizeT skip(SizeT length)
    {
        if (start_ + length <= length_)
        {
            start_ += length;
            return length;
        }

        SizeT distance = length_ - start_;
        start_ = length_;

        return distance;
    }

    virtual SizeT getStart() const
    {
        return start_;
    }

    virtual SizeT getRemainder() const
    {
        return length_ - start_;
    }

    virtual SizeT getSize() const
    {
        return length_;
    }

    virtual SizeT getAdvance() const
    {
        return getRemainder();
    }

    virtual void  reset(SizeT pos)
    {
        start_  = start0_;
    }

    virtual SizeT get(T* buffer, SizeT start, SizeT length)
    {
        auto syms = sequence_->symbols();

        MoveBits(syms, buffer, start_ * Bits, start * Bits, length * Bits);

        return skip(length);
    }

    virtual T get()
    {
        T value = sequence_->symbol(start_);

        skip(1);

        return value;
    }
};



template <typename Seq>
class SequenceDataTargetAdapter: public ISequenceDataTarget<typename Seq::Value, Seq::BitsPerSymbol> {

    static const SizeT Bits     = Seq::BitsPerSymbol;

    typedef typename Seq::Value T;

    SizeT   start0_;
    SizeT   start_;
    SizeT   length_;

    Seq*    sequence_;
public:

    SequenceDataTargetAdapter(Seq* sequence, SizeT start, SizeT length):
        start0_(start),
        start_(start),
        length_(length),
        sequence_(sequence)
    {}

    virtual SizeT skip(SizeT length)
    {
        if (start_ + length <= length_)
        {
            start_ += length;
            return length;
        }

        SizeT distance = length_ - start_;
        start_ = length_;

        return distance;
    }

    virtual SizeT getStart() const
    {
        return start_;
    }

    virtual SizeT getRemainder() const
    {
        return length_ - start_;
    }

    virtual SizeT getSize() const
    {
        return length_;
    }

    virtual SizeT getAdvance() const
    {
        return getRemainder();
    }

    virtual void  reset(SizeT pos)
    {
        start_  = start0_;
    }

    virtual SizeT put(const T* buffer, SizeT start, SizeT length)
    {
        auto syms = sequence_->symbols();

        MoveBits(buffer, syms, start * Bits, start_ * Bits, length * Bits);

        return skip(length);
    }

    virtual void put(const T& value)
    {
        sequence_->symbol(start_) = value;

        skip(1);
    }
};




template <Int BitsPerSymbol>
class PackedFSESequence {
protected:
    typedef PackedFSESequence<BitsPerSymbol>                                    MyType;

    typedef typename PkdFSSeqTF<BitsPerSymbol>::Type                            Types;
    typedef PkdFSSeq<Types>                                                     Seq;

    Seq* sequence_;

public:

    typedef typename Seq::Value                                                 Symbol;


    static const Int Bits                                                       = Seq::BitsPerSymbol;
    static const Int Symbols                                                    = Seq::Indexes;

    typedef ISequenceDataSource<Symbol, Bits>                                   IDataSrc;
    typedef ISequenceDataTarget<Symbol, Bits>                                   IDataTgt;
    typedef SequenceDataSourceAdapter<Seq>                                      SourceAdapter;
    typedef SequenceDataTargetAdapter<Seq>                                      TargetAdapter;

    typedef typename Seq::ConstSymbolAccessor                                   ConstSymbolAccessor;
    typedef typename Seq::SymbolAccessor                                        SymbolAccessor;

    typedef StaticVector<BigInt, Symbols>                                       Ranks;

    PackedFSESequence(Int capacity = 1024, Int density_hi = 1, Int density_lo = 1)
    {
        Int sequence_block_size = Seq::estimate_block_size(capacity, density_hi, density_lo);

        Int block_size = PackedAllocator::block_size(sequence_block_size, 1);

        PackedAllocator* alloc = T2T<PackedAllocator*>(malloc(block_size));
        alloc->init(block_size, 1);

        sequence_ = alloc->template allocateEmpty<Seq>(0);
    }

    ~PackedFSESequence()
    {
        if (sequence_)
        {
            free(sequence_->allocator());
        }
    }

    PackedFSESequence(const MyType& other)
    {
        const PackedAllocator* other_allocator = other.sequence_->allocator();

        Int block_size = other_allocator->block_size();

        PackedAllocator* allocator = T2T<PackedAllocator*>(malloc(block_size));

        CopyByteBuffer(other_allocator, allocator, block_size);

        sequence_ = allocator->template get<Seq>(0);
    }

    PackedFSESequence(MyType&& other)
    {
        sequence_ = other.sequence_;
        other.sequence_ = nullptr;
    }

    Int size() const
    {
        return sequence_->size();
    }

    void reindex()
    {
        sequence_->reindex();
    }

    void dump(std::ostream& out = std::cout) const
    {
        sequence_->dump(out);
    }

    SymbolAccessor operator[](Int idx)
    {
        return sequence_->symbol(idx);
    }

    ConstSymbolAccessor operator[](Int idx) const
    {
        return static_cast<const Seq*>(sequence_)->symbol(idx);
    }

    SourceAdapter source(Int idx, Int length) const
    {
        return SourceAdapter(sequence_, idx, length);
    }

    SourceAdapter source() const
    {
        return source(0, sequence_->size());
    }

    TargetAdapter target(Int idx, Int length) const
    {
        return TargetAdapter(sequence_, idx, length);
    }

    TargetAdapter target() const
    {
        return target(0, sequence_->size());
    }

    void update(Int start, IDataSrc& src)
    {
        Symbol* syms = sequence_->symbols();
        src.get(syms, start, src.getSize());
    }

    void insert(Int at, IDataSrc& src)
    {
        //sequence_->insert(&src, at, src.getRemainder());
    }

    void insert(Int at, Int symbol)
    {
        sequence_->insert(at, symbol);
    }

    void insert(Int at, Int length, std::function<Symbol ()> fn)
    {
        sequence_->insert(at, length, fn);
    }

    void remove(Int start, Int end)
    {
        sequence_->remove(start, end);
    }

    void append(IDataSrc& src)
    {
//        Int at = sequence_->size();
        //sequence_->insert(&src, at, src.getRemainder());
    }

    void append(Int length, std::function<Symbol ()> fn)
    {
        sequence_->insert(sequence_->size(), length, fn);
    }

    void append(Symbol symbol)
    {
        Int size = sequence_->size();
        sequence_->insert(size, symbol);
    }

    void read(Int from, IDataTgt& tgt) const
    {
        const Symbol* syms = sequence_->symbols();
        tgt.put(syms, from, tgt.getSize());
    }

    Int rank(Int end, Int symbol) const {
        return sequence_->rank(end, symbol);
    }

    Int rank(Int start, Int end, Int symbol) const {
        return sequence_->rank(start, end, symbol);
    }

    SelectResult select(Int symbol, Int rank) const {
        return sequence_->selectFw(symbol, rank);
    }

    SelectResult selectFw(Int start, Int symbol, Int rank) const {
        return sequence_->selectFw(start, symbol, rank);
    }

    SelectResult selectBw(Int symbol, Int rank) const {
        return sequence_->selectFw(symbol, rank);
    }

    SelectResult selectBw(Int start, Int symbol, Int rank) const {
        return sequence_->selectBw(start, symbol, rank);
    }

    Ranks ranks() const
    {
        return ranks(size());
    }

    Ranks ranks(Int to) const
    {
        auto s_ranks = sequence_->sums(to);
        Ranks ranks;
        ranks.assignDown(s_ranks);

        return ranks;
    }

    Ranks ranks(Int from, Int to) const
    {
        auto s_ranks = sequence_->sums(from, to);
        Ranks ranks;
        ranks.assignDown(s_ranks);

        return ranks;
    }
};

}}