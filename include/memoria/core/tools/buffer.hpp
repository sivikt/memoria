
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CORE_TOOLS_BUFFER_H
#define _MEMORIA_CORE_TOOLS_BUFFER_H


#include <memoria/core/types/types.hpp>
#include <memoria/core/tools/bitmap.hpp>
#include <memoria/core/types/typehash.hpp>

#include <iostream>



namespace memoria    {

#pragma pack(1)

//typedef long bitmap_atom_t;
//
//template <long Size, long Align = sizeof(long)>
//class Padding {
//    static const long SIZE = (sizeof(Size) % Align == 0 ? Align : (sizeof(bitmap_atom_t) - Size % sizeof(bitmap_atom_t))) ;
//    char bytes[SIZE];
//public:
//    Padding(){}
//};

//template <typename T>
//class Wrapper {
//    T value_;
//public:
//    static const int SIZE                           = sizeof(T);
//    static const int BITSIZE                        = sizeof(T) * 8;
//
//    typedef T                                       ValueType;
//
//    Wrapper(const T &value): value_(value) {};
//    Wrapper(const Wrapper<T> &copy): value_(copy.value_) {};
//
//    T &value() const {
//        return value_;
//    }
//
//    void set_value(const T &v) {
//        value_ = v;
//    }
//
//    bool operator==(const Wrapper<T> &v) const {
//        return value_ == v.value_;
//    }
//
//    bool operator!=(const Wrapper<T> &v) const {
//        return value_ != v.value_;
//    }
//
//    bool operator>=(const Wrapper<T> &v) const {
//        return value_ >= v.value_;
//    }
//
//    bool operator<=(const Wrapper<T> &v) const {
//        return value_ <= v.value_;
//    }
//
//    bool operator<(const Wrapper<T> &v) const {
//        return value_ < v.value_;
//    }
//};



template <size_t Size>
class StaticBuffer {

    char Buffer_[Size];

    typedef StaticBuffer<Size> Me;

public:
    typedef Long            Element;

    static const BigInt     SIZE    = Size;                 //in bytes;
    static const BigInt     BITSIZE = SIZE * 8;             //in bits;

    StaticBuffer() {}

    const Me& operator=(const Me& other) {
        CopyBuffer(other.Buffer_, Buffer_, Size);
        return *this;
    }

    bool operator==(const Me&other) const {
        return CompareBuffers(Buffer_, other.Buffer_, Size);
    }

    bool operator!=(const Me&other) const {
        return !CompareBuffers(Buffer_, other.Buffer_, Size);
    }

    const char *ptr() const {
        return Buffer_;
    }

    char *ptr() {
        return Buffer_;
    }

    void clear() {
        for (Int c = 0; c < (Int)Size; c++) {
            Buffer_[c] = 0;
        }
    }

    static bool isVoid() {
        return SIZE == 0;
    }

    const Long &operator[](Int idx) const {
        return *(CP2CP<Long>(ptr()) + idx);
    }

    Long &operator[](Int idx) {
        return *(T2T<Long*>(ptr()) + idx);
    }

    void copyFrom(const void *mem) {
        CopyBuffer(mem, Buffer_, Size);
    }

    void copyTo(void *mem) const {
        CopyBuffer(Buffer_, mem, Size);
    }

    void dump(std::ostream &os, Int size = BITSIZE) {
        dump(os, *this, (Int)0, size);
    }

    Int getHashCode() {
        return PtrToInt(ptr());
    }
};


template <typename Object>
class ValueBuffer {
    typedef ValueBuffer<Object>                                                 MyType;
    Object value_;

public:

    static const BigInt     SIZE    = sizeof(Object);       //in bytes;
    static const BigInt     BITSIZE = SIZE * 8;             //in bits;

    typedef Object                                                              ValueType;

    ValueBuffer() {}

    ValueBuffer(const Object &obj) {
        value() = obj;
    }

    void clear() {
        value_ = 0;
    }

    void copyTo(void *mem) const {
        CopyByteBuffer(&value_, mem, sizeof(Object));
    }

    void copyFrom(const void *mem) {
        CopyByteBuffer(mem, &value_, sizeof(Object));
    }

    const Object &value() const {
        return value_;
    }

    Object &value() {
        return value_;
    }


    bool operator<(const MyType &other) const {
        return value() < other.value();
    }
};

class VoidBuffer {
public:

    static const int SIZE = 0;              //in bytes;
    static const int BITSIZE = SIZE * 8;    //in bits;

    VoidBuffer() {}

    static bool is_void() {
        return true;
    }

    const char* ptr() const {
        return NULL;
    }

    char* ptr() {
        return NULL;
    }

    Int getHashCode() {
        return 0;
    }
};

#pragma pack()

} //memoria



#endif
