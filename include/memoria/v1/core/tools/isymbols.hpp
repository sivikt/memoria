
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once


#include <memoria/v1/core/tools/config.hpp>
#include <memoria/v1/core/tools/bitmap.hpp>
#include <memoria/v1/core/tools/assert.hpp>
#include <memoria/v1/core/tools/accessors.hpp>

#include <memoria/v1/core/types/types.hpp>
#include <memoria/v1/core/types/type2type.hpp>

#include <vector>
#include <iostream>
#include <vector>
#include <tuple>

#include <malloc.h>

namespace memoria {

template <typename T1, typename T2>
constexpr static T2 DivUp0(T1 v, T2 d) {
    return v / d + (v % d > 0);
}

template <typename T, Int BitsPerSymbol>
SizeT RoundSymbolsToStorageType(SizeT length)
{
    SizeT bitsize               = length * BitsPerSymbol;
    const SizeT item_bitsize    = sizeof(T) * 8;

    SizeT result = DivUp0(bitsize, item_bitsize);

    return result * sizeof(T);
};


namespace {

    template <Int BitsPerSymbol>
    struct SymbolsTypeSelector {
        using Type = UBigInt;
    };

    template <>
    struct SymbolsTypeSelector<8> {
        using Type = UByte;
    };

}


//
enum class IDataAPI: Int {
    None = 0, Batch = 1, Single = 2, Both = 3
};

inline constexpr IDataAPI operator&(IDataAPI m1, IDataAPI m2)
{
    return static_cast<IDataAPI>(static_cast<Int>(m1) & static_cast<Int>(m2));
}

inline constexpr IDataAPI operator|(IDataAPI m1, IDataAPI m2)
{
    return static_cast<IDataAPI>(static_cast<Int>(m1) | static_cast<Int>(m2));
}

inline constexpr bool to_bool(IDataAPI mode) {
    return static_cast<Int>(mode) > 0;
}



template <typename T>
class AbstractData {
protected:
    SizeT   start_;
    SizeT   length_;

public:

    AbstractData(SizeT start, SizeT length):
        start_(start),
        length_(length)
    {}

    virtual ~AbstractData() throw ()
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

    virtual SizeT getAdvance() const
    {
        return getRemainder();
    }

    virtual SizeT getSize() const
    {
        return length_;
    }

    SizeT size() const
    {
        return getSize();
    }

    virtual void setSize(SizeT size)
    {
        length_ = size;
    }

    virtual IDataAPI api() const                                                = 0;
    virtual SizeT put(const T* buffer, SizeT start, SizeT length)               = 0;
    virtual SizeT get(T* buffer, SizeT start, SizeT length)                     = 0;
    virtual T peek()                                                            = 0;


    virtual T get()                                                             = 0;
    virtual void put(const T& value)                                            = 0;

    virtual void reset(BigInt pos = 0)
    {
        start_ = pos;
    }
};


template <typename T>
class AbstractDataSource {
protected:
    SizeT   start_;
    SizeT   length_;

public:

    AbstractDataSource(SizeT start, SizeT length):
        start_(start),
        length_(length)
    {}

    virtual ~AbstractDataSource() throw ()
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

    virtual SizeT getAdvance() const
    {
        return getRemainder();
    }

    virtual SizeT getSize() const
    {
        return length_;
    }

    SizeT size() const
    {
        return getSize();
    }

    virtual void setSize(SizeT size)
    {
        length_ = size;
    }

    virtual IDataAPI api() const                                                = 0;

    virtual SizeT get(T* buffer, SizeT start, SizeT length)                     = 0;
    virtual T get()                                                             = 0;


    virtual void reset(BigInt pos = 0)
    {
        start_ = pos;
    }
};




template <Int BitsPerSymbol, typename T = typename SymbolsTypeSelector<BitsPerSymbol>::Type>
class SymbolsBuffer: public AbstractData<T> {
    typedef SymbolsBuffer<BitsPerSymbol, T>                                     MyType;
protected:
    typedef AbstractData<T>                                                     Base;

    T*      data_;
    bool    owner_;
public:

    typedef T value_type;

    static SizeT storage_size(SizeT length)
    {
        return RoundSymbolsToStorageType<T, BitsPerSymbol>(length);
    }

    SymbolsBuffer(T* data, SizeT length, bool owner = false):
        Base(0, length),
        data_(data),
        owner_(owner)
    {}

    SymbolsBuffer(SizeT length):
        Base(0, length),
        data_(T2T<T*>(length > 0 ?::malloc(storage_size(length)) : nullptr)),
        owner_(true)
    {}

    SymbolsBuffer(MyType&& other):
        Base(other.start_, other.length_),
        data_(other.data_),
        owner_(other.owner_)
    {
        other.data_ = NULL;
    }

    SymbolsBuffer(const MyType& other):
        Base(other.start, other.length_),
        owner_(true)
    {
        SizeT ssize = storage_size(this->length_);

        data_ = T2T<T*>(this->length_ > 0 ?::malloc(ssize) : nullptr);

        CopyBuffer(other.data(), data_, ssize);
    }

    virtual ~SymbolsBuffer() throw ()
    {
        if (owner_) {
            ::free(data_);
        }
    }

    virtual IDataAPI api() const
    {
        return IDataAPI::Both;
    }

    T* data()
    {
        return data_;
    }

    const T* data() const
    {
        return data_;
    }

    SizeT size() const
    {
        return this->getSize();
    }


    virtual SizeT put(const T* buffer, SizeT start, SizeT length)
    {
        MEMORIA_ASSERT_TRUE(this->start_ + length <= this->length_);

        MoveBits(buffer, data_, start * BitsPerSymbol, this->start_ * BitsPerSymbol, length * BitsPerSymbol);

        return this->skip(length);
    }

    virtual SizeT putc(const T* buffer, SizeT buf_start, SizeT start, SizeT length)
    {
        MEMORIA_ASSERT_TRUE(this->start_ + buf_start + length <= this->length_);

        MoveBits(buffer, data_, start * BitsPerSymbol, (this->start_ + buf_start) * BitsPerSymbol, length * BitsPerSymbol);

        return length;
    }


    virtual SizeT get(T* buffer, SizeT start, SizeT length)
    {
        MEMORIA_ASSERT_TRUE(this->start_ + length <= this->length_);

        MoveBits(data_, buffer, this->start_ * BitsPerSymbol, start * BitsPerSymbol, length * BitsPerSymbol);

        return this->skip(length);
    }

    virtual SizeT getc(T* buffer, SizeT buf_start, SizeT start, SizeT length) const
    {
        MEMORIA_ASSERT_TRUE(this->start_ + buf_start + length <= this->length_);

        MoveBits(data_, buffer, (this->start_ + buf_start) * BitsPerSymbol, start * BitsPerSymbol, length * BitsPerSymbol);

        return length;
    }

    virtual T get()
    {
        T value = GetBits(data_, this->start_ * BitsPerSymbol, BitsPerSymbol);

        this->skip(1);

        return value;
    }

    virtual T peek() {
        return GetBits(data_, this->start_ * BitsPerSymbol, BitsPerSymbol);
    }

    virtual void put(const T& value)
    {
        SetBits(data_, this->start_ * BitsPerSymbol, value, BitsPerSymbol);
        this->skip(1);
    }


    void dump(std::ostream& out) const
    {
        dumpSymbols(out, data_, this->length_, BitsPerSymbol);
    }

    void clear()
    {
        SizeT len = storage_size(this->length_) / sizeof(T);

        for (SizeT c = 0; c < len; c++)
        {
            data_[c] = 0;
        }
    }

    BitmapAccessor<T*, T, BitsPerSymbol> operator[](SizeT idx)
    {
        return BitmapAccessor<T*, T, BitsPerSymbol>(data_, idx);
    }

    BitmapAccessor<const T*, T, BitsPerSymbol> operator[](SizeT idx) const
    {
        return BitmapAccessor<const T*, T, BitsPerSymbol>(data_, idx);
    }
};







template <Int BitsPerSymbol, typename T>
class SymbolsBuffer<BitsPerSymbol, const T>: public AbstractDataSource<T> {
    typedef SymbolsBuffer<BitsPerSymbol, T>                                     MyType;

protected:
    typedef AbstractDataSource<T>                                               Base;

    const T*    data_;
public:

    static SizeT storage_size(SizeT length)
    {
        return RoundSymbolsToStorageType<T, BitsPerSymbol>(length);
    }

    SymbolsBuffer(const T* data, SizeT length):
        Base(0, length),
        data_(data)
    {}

    SymbolsBuffer(const MyType& other):
        Base(other.start_, other.length_),
        data_(other.data_)
    {}


    virtual ~SymbolsBuffer() throw () {}

    virtual IDataAPI api() const
    {
        return IDataAPI::Both;
    }

    const T* data() const
    {
        return data_;
    }

    SizeT size() const {
        return this->getSize();
    }


    virtual SizeT get(T* buffer, SizeT start, SizeT length)
    {
        MoveBits(data_, buffer, this->start_ * BitsPerSymbol, start * BitsPerSymbol, length * BitsPerSymbol);

        return this->skip(length);
    }

    virtual T get()
    {
        T value = GetBits(data_, this->start_ * BitsPerSymbol, BitsPerSymbol);

        this->skip(1);

        return value;
    }

    void dump(std::ostream& out) const
    {
        dumpSymbols(out, data_, this->length_, BitsPerSymbol);
    }

    virtual void reset(SizeT pos = 0)
    {
        this->start_ = pos;
    }
};


}
