
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CORE_CONTAINER_PAGE_HPP
#define	_MEMORIA_CORE_CONTAINER_PAGE_HPP

#include <memoria/metadata/metadata.hpp>


#include <memoria/core/tools/buffer.hpp>
#include <memoria/vapi/models/logs.hpp>

namespace memoria    {

template <typename T, size_t Size = sizeof(T)>
class AbstractPageID: public ValueBuffer<T, Size> {
public:
    typedef AbstractPageID<T, Size>                 ValueType;
    typedef ValueBuffer<T, Size>                 	Base;

    AbstractPageID(): Base() {}

    AbstractPageID(const T &t) : Base(t) {}

    AbstractPageID(const IDValue& id): Base() {
        Base::CopyFrom(id.ptr());
    }

    bool is_null() const {
        return IsClean(Base::ptr(), Size);
    }

    bool is_not_null() const {
    	return !is_null();
    }

    void set_null() {
        Clean(Base::ptr(), Size);
    }
};

template <typename T, size_t Size>
static LogHandler& operator<<(LogHandler &log, const AbstractPageID<T, Size>& value)
{
    IDValue id(&value);
    log.log(id);
    log.log(" ");
    return log;
}

template <typename T, size_t Size>
static LogHandler* LogIt(LogHandler* log, const AbstractPageID<T, Size>& value)
{
    IDValue id(&value);
    log->log(id);
    log->log(" ");
    return log;
}


template <Int Size>
class BitBuffer: public Buffer<Size % 8 == 0 ? Size / 8 : ((Size / 8) + 1)> {
    typedef Buffer<(Size % 8 == 0 ? Size / 8 : ((Size / 8) + 1))>               Base;
public:

    typedef Int                         Index;
    typedef Long                        Bits;

    static const Int kBitSize           = Size;

    static const int RESERVED_SIZE      = 0;
    static const int RESERVED_BITSIZE   = RESERVED_SIZE * 8;

    BitBuffer() : Base() {}

    bool is_bit(Index index) const {
        return GetBit(*this, index + RESERVED_BITSIZE);
    }

    Bits get_bits(Index idx, Index count) const {
        return GetBits(*this, idx, count);
    }

    void set_bits(Index idx, Bits bits, Index count) {
        SetBits(*this, idx, bits, count);
    }

    void set_bit(int index, int bit) {
        SetBit(*this, index + RESERVED_BITSIZE, bit);
    }
};

template <typename PageIdType, Int FlagsCount = 32>
class AbstractPage {

    typedef AbstractPage<PageIdType, FlagsCount> Me;

    typedef BitBuffer<FlagsCount> FlagsType;

    FlagsType   flags_;
    PageIdType  id_;

    Int         crc_;
    Int         model_hash_;
    Int         page_type_hash_;

public:
    typedef PageIdType      ID;

    AbstractPage(): flags_(), id_() {}

    AbstractPage(const PageIdType &id): flags_(), id_(id) {}

    const PageIdType &id() const {
        return id_;
    }

    PageIdType &id() {
        return id_;
    }

    const FlagsType &flags() const {
        return flags_;
    };

    void init() {}

    FlagsType &flags() {
        return flags_;
    };

    Int &crc() {
        return crc_;
    }

    const Int &crc() const {
        return crc_;
    }

    Int &model_hash() {
        return model_hash_;
    }

    const Int &model_hash() const {
        return model_hash_;
    }

    Int &page_type_hash() {
        return page_type_hash_;
    }

    const Int &page_type_hash() const {
        return page_type_hash_;
    }

    Int data_size() const {
        return sizeof(Me);
    }

    void *operator new(size_t size, void *page) {
        return page;
    }

    void operator delete(void *buf) {}

    //Rebuild page content such indexes using provided data.
    void Rebiuild(){}

    template <template <typename> class FieldFactory>
    void BuildFieldsList(MetadataList &list, Long &abi_ptr) const {
        FieldFactory<PageIdType>::create(list, id(),        "ID",               abi_ptr);
        FieldFactory<FlagsType>::create(list,  flags(),     "FLAGS",            abi_ptr);
        FieldFactory<Int>::create(list,  crc(),             "CRC",              abi_ptr);
        FieldFactory<Int>::create(list,  model_hash(),      "MODEL_HASH",       abi_ptr);
        FieldFactory<Int>::create(list,  page_type_hash(),  "PAGE_TYPE_HASH",   abi_ptr);
    }

    template <typename PageType>
    void CopyFrom(PageType* page)
    {
        this->id()              = page->id();
        this->flags()           = page->flags();
        this->crc()             = page->crc();
        this->model_hash()      = page->model_hash();
        this->page_type_hash()  = page->page_type_hash();
    }
};

}

#endif
