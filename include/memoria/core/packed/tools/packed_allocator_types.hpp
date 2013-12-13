
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_CORE_PACKED_ALLOCATOR_TYPES_HPP_
#define MEMORIA_CORE_PACKED_ALLOCATOR_TYPES_HPP_

#include <memoria/core/types/types.hpp>
#include <memoria/core/types/type2type.hpp>
#include <memoria/core/exceptions/memoria.hpp>
#include <memoria/core/tools/reflection.hpp>
#include <memoria/metadata/page.hpp>

namespace memoria {


class MEMORIA_API PackedOOMException: public vapi::MemoriaThrowable {

    Int total_;
    Int requested_;
    Int free_;

    const char* msg_;

public:

    PackedOOMException(const char* source, Int total, Int requested, Int free):
                MemoriaThrowable(source ),
                total_(total),
                requested_(requested),
                free_(free),
                msg_(nullptr)
    {}

    PackedOOMException(const char* source, const char* msg):
        MemoriaThrowable(source ),
        msg_(msg)
    {}

    virtual void dump(ostream& out) const
    {
        if (msg_ != nullptr) {
            out<<"PackedOOMException at "<<source_<<": "<<msg_;
        }
        else {
            out<<"PackedOOMException at "<<source_<<": Total="<<total_<<" Requested="<<requested_<<" Free="<<free_;
        }
    }
};


template <typename MyType, typename Base> class PackedAllocatorBase;

class PackedAllocator;

class PackedAllocatable {
protected:
    Int allocator_offset_;

    Int& allocator_offset() {return allocator_offset_;}

public:

    static const UInt VERSION                   = 1;
    static const Int AlignmentBlock             = PackedAllocationAlignment;


    typedef TypeList<
            ConstValue<UInt, VERSION>,
            decltype(allocator_offset_)
    >                                           FieldsList;

    PackedAllocatable() = default;

    const Int& allocator_offset() const {return allocator_offset_;}

    template <typename MyType, typename Base>
    friend class PackedAllocatorBase;

    friend class PackedAllocator;

    bool has_allocator() const
    {
        return allocator_offset_ > 0;
    }

    void setAllocatorOffset(const void* allocator)
    {
        const char* my_ptr = T2T<const char*>(this);
        const char* alc_ptr = T2T<const char*>(allocator);
        size_t diff = T2T<size_t>(my_ptr - alc_ptr);
        allocator_offset() = diff;
    }

    PackedAllocator* allocator()
    {
        if (allocator_offset() > 0)
        {
            UByte* my_ptr = T2T<UByte*>(this);
            return T2T<PackedAllocator*>(my_ptr - allocator_offset());
        }
        else {
            throw PackedOOMException(MA_SRC, "No allocation is defined for this object");
        }
    }

    const PackedAllocator* allocator() const
    {
        if (allocator_offset() > 0)
        {
            const UByte* my_ptr = T2T<const UByte*>(this);
            return T2T<const PackedAllocator*>(my_ptr - allocator_offset());
        }
        else {
            throw PackedOOMException(MA_SRC, "No allocation is defined for this object");
        }
    }

    static Int roundUpBytesToAlignmentBlocks(Int value)
    {
        return (value / AlignmentBlock + (value % AlignmentBlock ? 1 : 0)) * AlignmentBlock;
    }

    static Int roundDownBytesToAlignmentBlocks(Int value)
    {
        return (value / AlignmentBlock) * AlignmentBlock;
    }

    static Int roundUpBitsToAlignmentBlocks(Int bits)
    {
        return roundUpBytesToAlignmentBlocks(roundUpBitToBytes(bits));
    }

    static Int roundDownBitsToAlignmentBlocks(Int bits)
    {
        return roundDownBytesToAlignmentBlocks(roundDownBitsToBytes(bits));
    }

    static Int roundUpBitToBytes(Int bits)
    {
        return bits / 8 + (bits % 8 > 0);
    }

    static Int roundDownBitsToBytes(Int bits)
    {
        return bits / 8 + (bits % 8 > 0);
    }

    void serialize(SerializationData& buf) const
    {
        FieldFactory<Int>::serialize(buf, allocator_offset_);
    }

    void deserialize(DeserializationData& buf)
    {
        FieldFactory<Int>::deserialize(buf, allocator_offset_);
    }

};

template <Int Alignment = PackedAllocationAlignment>
struct PackedAllocatorTypes {
    static const Int AllocationAlignment = Alignment;
};


struct AllocationBlock {
    Int size_;
    Int offset_;
    UByte* ptr_;

    AllocationBlock(Int size, Int offset, UByte* ptr): size_(size), offset_(offset), ptr_(ptr) {}

    Int size() const    {return size_;}
    Int offset() const  {return offset_;}
    UByte* ptr() const  {return ptr_;}
    bool is_empty() const {return size_ == 0;}

    template <typename T>
    const T* cast() const {
        return T2T<const T*>(ptr_);
    }

    template <typename T>
    T* cast() {
        return T2T<T*>(ptr_);
    }
};


struct AllocationBlockConst {
    Int size_;
    Int offset_;
    const UByte* ptr_;

    AllocationBlockConst(Int size, Int offset, const UByte* ptr): size_(size), offset_(offset), ptr_(ptr) {}

    Int size() const    {return size_;}
    Int offset() const  {return offset_;}
    const UByte* ptr() const    {return ptr_;}

    operator bool() const {return true;}

    template <typename T>
    const T* cast() const {
        return T2T<const T*>(ptr_);
    }
};

struct EmptyAllocator {
    Int enlargeBlock(void*, Int size) {return 0;}

    static Int roundUpBytesToAlignmentBlocks(int size) {return size;}
};

}


#endif
