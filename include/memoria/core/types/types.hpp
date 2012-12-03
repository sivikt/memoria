
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CORE_TYPES_TYPES_HPP
#define _MEMORIA_CORE_TYPES_TYPES_HPP

#include <string>
#include <sstream>
#include <cassert>
#include <stdint.h>

#include <memoria/core/tools/config.hpp>

namespace memoria    {

static const int MAX_BLOCK_SIZE = 4096;

typedef int64_t             BigInt;
typedef uint64_t            UBigInt;
typedef int32_t             Int;
typedef uint32_t            UInt;
typedef int16_t             Short;
typedef uint16_t            UShort;
typedef int8_t              Byte;
typedef uint8_t             UByte;
typedef size_t              SizeT;


namespace internal {
template <int size> struct PlatformLongHelper;

template <>
struct PlatformLongHelper<4> {
    typedef Int             LongType;
    typedef UInt            ULongType;
};

template <>
struct PlatformLongHelper<8> {
    typedef BigInt          LongType;
    typedef UBigInt         ULongType;
};
}

/**
 * Please note that Long/ULong types are not intended to be used for data page properties.
 * Use types with known size instead.
 */

typedef internal::PlatformLongHelper<sizeof(void*)>::LongType                   Long;
typedef internal::PlatformLongHelper<sizeof(void*)>::ULongType                  ULong;

typedef std::string                                                             String;
typedef const String&                                                           StringRef;

template <typename T, T V> struct ConstValue {
    static const T Value = V;
};

template <UInt Value>
using UIntValue = ConstValue<UInt, Value>;

template <typename T> struct TypeHash; // must define Value constant

/*
 * Container type names & profiles
 */

struct BTreeCtr     {};
struct BSTreeCtr    {};
struct DynVectorCtr {};
struct CompositeCtr {};

struct Superblock   {};
struct RootCtr      {};

template <Int Indexes>
struct MapCtr       {};

typedef MapCtr<1>       Map1Ctr;

template <Int Indexes>
struct SetCtr       {};

typedef SetCtr<1>       Set1Ctr;
typedef SetCtr<2>       Set2Ctr;

struct DFUDS        {};
struct LOUDS        {};
struct VectorMapCtr {};
struct VectorCtr    {};


template <typename ChildType = void>
class SmallProfile {};

/*
 * End of container type names and profiles
 */




struct NullType {};

struct EmptyType {};
struct IncompleteType;

template <typename Name>
struct TypeNotFound;
struct TypeIsNotDefined;

template <typename FirstType, typename SecondType>
struct Pair {
    typedef FirstType   First;
    typedef SecondType  Second;
};

//template <BigInt Code>
//struct TypeCode {
//    static const BigInt Value = Code;
//};


struct TrueValue {
    static const bool Value = true;
};

struct FalseValue {
    static const bool Value = false;
};

template <typename T>
struct TypeDef {
    typedef T Type;
};

class NotDefined {};

template <int Value>
struct CtrNameDeclarator: TypeDef<NotDefined> {};

template <typename ... Types>
struct TypeList {};

template <typename T, T ... Values>
struct ValueList {};

template <typename Type_, Type_ V>
struct ValueWrapper {
    typedef Type_               Type;
    static const Type Value     = V;
};

class EmptyValue {
public:
    EmptyValue() {}
    EmptyValue(const BigInt) {}
    EmptyValue(const EmptyValue& other) {}
    EmptyValue& operator=(const EmptyValue& other) {
        return *this;
    }

    template <typename T>
    operator T () {
        return 0;
    }

    operator BigInt () const {
        return 0;
    }
};

template <typename First, typename Second>
struct ValuePair {

    First   first;
    Second  second;

    ValuePair(const First& f, const Second& s): first(f), second(s) {}
    ValuePair(const First& f): first(f) {}
};

struct NoParamCtr {};
struct IterEndMark {};

struct SerializationData {
    char* buf;
    Int total;
};

struct DeserializationData {
    const char* buf;
};

extern BigInt DebugCounter;

}

#endif
