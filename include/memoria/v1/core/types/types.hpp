
// Copyright 2011 Victor Smirnov
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

#include <memoria/v1/core/tools/config.hpp>

#include <string>
#include <sstream>
#include <cassert>
#include <cstdint>

#ifdef _MSC_VER
#include <windows.h>
//#include <intrin.h>

#pragma warning( disable : 4267)
#pragma warning( disable : 4146)
#pragma warning( disable : 4307)
#pragma warning( disable : 4200)
#pragma warning( disable : 4291)
#pragma warning( disable : 4101)
#pragma warning( disable : 4305)
#pragma warning( disable : 4018)
#pragma warning( disable : 4805)


#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#endif

#include <type_traits>
#include <tuple>

namespace memoria {
namespace v1 {

static constexpr int DEFAULT_BLOCK_SIZE                 = 8192;
static constexpr int PackedTreeBranchingFactor          = 32;
static constexpr int PackedSeqBranchingFactor           = 32;
static constexpr int PackedSeqValuesPerBranch           = 1024;
static constexpr int PackedTreeExintVPB                 = 256;
static constexpr int PackedTreeEliasVPB                 = 1024;
static constexpr int PackedAllocationAlignment          = 8;

static constexpr size_t MaxRLERunLength                 = 0x7FFFFFF;

typedef std::int64_t            BigInt;
typedef std::uint64_t           UBigInt;
typedef std::int32_t            Int;
typedef std::uint32_t           UInt;
typedef std::int16_t            Short;
typedef std::uint16_t           UShort;
typedef char                    Char;
typedef std::int8_t             Byte;
typedef std::uint8_t            UByte;

enum class PackedSizeType {FIXED, VARIABLE};

namespace internal {
    template <int size> struct PlatformLongHelper;

    template <>
    struct PlatformLongHelper<4> {
        typedef Int             LongType;
        typedef UInt            ULongType;
        typedef Int             SizeTType;
    };

    template <>
    struct PlatformLongHelper<8> {
        typedef BigInt          LongType;
        typedef UBigInt         ULongType;
        typedef BigInt          SizeTType;
    };
}



enum {
    CTR_NONE                = 0,
    CTR_CREATE              = 1,
    CTR_FIND                = 1<<1,
//    CTR_THROW_IF_EXISTS     = 1<<2,
};

/**
 * Please note that Long/ULong types are not intended to be used for data page properties.
 * Use types with known size instead.
 */

typedef internal::PlatformLongHelper<sizeof(void*)>::LongType                   Long;
typedef internal::PlatformLongHelper<sizeof(void*)>::ULongType                  ULong;
typedef internal::PlatformLongHelper<sizeof(void*)>::SizeTType                  SizeT;



template <typename T, T V> struct ConstValue {
    static constexpr T Value = V;

    constexpr operator T() const noexcept {return Value;}
    constexpr T operator()() const noexcept {return Value;}
};

template <UInt Value>
using UIntValue = ConstValue<UInt, Value>;

template <Int Value>
using IntValue = ConstValue<Int, Value>;


template <bool Value>
using BoolValue = ConstValue<bool, Value>;


template <typename...> using VoidT = void;

class EmptyValue {
public:
    EmptyValue() {}
    EmptyValue(const Int) {}
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

template <typename T> struct TypeHash; // must define Value constant


template <typename ... Types>
struct TypeList {
    constexpr TypeList() = default;
};

template <typename ... Types>
using TL = TypeList<Types...>;


template <typename T, T ... Values>
struct ValueList {
    constexpr ValueList() = default;
};


template <Int... Values>
using IntList = ValueList<Int, Values...>;

template <Int... Values>
using UIntList = ValueList<UInt, Values...>;



/*
 * Container type names & profiles
 */

struct BT {};



struct Composite    {};
struct Root         {};

template <typename CtrName>
class CtrWrapper    {};

template <typename Key, typename Value>
struct Map          {};

template <typename Key, typename Value>
struct CowMap       {};

template <typename Key, typename Value, PackedSizeType SizeType>
struct Table        {};




template <typename Key>
struct Set          {};



template <typename T>
struct Vector       {};

template <Int BitsPerSymbol, bool Dense = true>
struct Sequence {};

template <bool Dense = true>
using BitVector = Sequence<1, Dense>;

template <typename ChildType = void>
class DefaultProfile  {};


enum class Granularity  {Bit, Byte};
enum class Indexed      {No, Yes};

template <
    typename LblType,
    Indexed indexed         = Indexed::No
>
struct FLabel       {};

template <
    Int BitsPerSymbol
>
struct FBLabel      {};

template <
    typename LblType,
    Granularity granularity = Granularity::Bit,
    Indexed indexed         = Indexed::No
>
struct VLabel       {};


template <typename... LabelDescriptors>
struct LabeledTree  {};


struct WT           {};
struct VTree        {};

template <Granularity granularity, typename T = BigInt>
struct VLen {};

template <Granularity gr = Granularity::Byte>
using CMap = Map<VLen<gr>, BigInt>;

template <Granularity gr = Granularity::Byte>
using CCMap = Map<VLen<gr>, VLen<gr>>;



// Placeholder type to be used in place of Page IDs
struct IDType {};


/*
 * End of container type names and profiles
 */

/*
 * Prototype names
 */

class Tree {};

/*
 * End of prototype names
 */

struct NullType {};

struct EmptyType {};

struct EmptyType1 {};
struct EmptyType2 {};

struct IncompleteType;
struct TypeIsNotDefined {};

template <typename Name>
struct TypeNotFound;
struct TypeIsNotDefined;

template <typename FirstType, typename SecondType>
struct Pair {
    typedef FirstType   First;
    typedef SecondType  Second;
};


template <typename T>
struct TypeDef {
    typedef T Type;
};

class NotDefined {};

// For metadata initialization
template <int Order>
struct CtrNameDeclarator: TypeDef<NotDefined> {};









template <typename T> struct DeclType {
    typedef T Type;
};

template <typename First, typename Second>
struct ValuePair {

    First   first;
    Second  second;

    ValuePair(const First& f, const Second& s): first(f), second(s) {}
    ValuePair(const First& f): first(f) {}
    ValuePair() {}
};


struct IterEndMark {};

struct SerializationData {
    char* buf;
    Int total;

    SerializationData(): buf(nullptr), total(0) {}
};

struct DeserializationData {
    const char* buf;
};

enum class WalkDirection {
    UP, DOWN
};

enum class WalkCmd {
    FIRST_LEAF, LAST_LEAF, THE_ONLY_LEAF, FIX_TARGET, NONE, PREFIXES, REFRESH
};


enum class UpdateType {
    SET, ADD
};

enum class SearchType {LT, LE, GT, GE};
enum class IteratorMode {FORWARD, BACKWARD};
enum class MergeType {NONE, LEFT, RIGHT};
enum class MergePossibility {YES, NO, MAYBE};

enum class LeafDataLengthType {FIXED, VARIABLE};

template <typename T>
struct TypeP {
    using Type = T;
};

class NoParamCtr {};


class VLSelector {};
class FLSelector {};

enum class SplitStatus {NONE, LEFT, RIGHT, UNKNOWN};

class SplitResult {
    SplitStatus type_;
    Int idx_;
public:
    SplitResult(SplitStatus type, Int idx): type_(type), idx_(idx) {}
    SplitResult(SplitStatus type): type_(type), idx_() {}

    SplitStatus type() const {return type_;}
    Int idx() const {return idx_;}
};


template <typename PkdStruct>
struct IndexesSize {
    static const Int Value = PkdStruct::Indexes;
};


extern BigInt DebugCounter;
extern BigInt DebugCounter1;
extern BigInt DebugCounter2;

template <typename T>
using IL = std::initializer_list<T>;

template <typename List>  struct AsTupleH;

template <typename List>
using AsTuple = typename AsTupleH<List>::Type;

template <typename... Types>
struct AsTupleH<TL<Types...>> {
    using Type = std::tuple<Types...>;
};


template <typename T>
struct HasType {
    using Type = T;
};

template <typename T, T V_>
struct HasValue {
    static constexpr T Value = V_;
};


namespace details {
    template <typename T, bool Flag, typename T2>
    struct FailIfT {
        static_assert(!Flag, "Template failed");
        using Type = T;
    };
}

template <typename T, bool Flag = true, typename T2 = void>
using FailIf = typename v1::details::FailIfT<T, Flag, T2>::Type;

template <Int V, bool Flag = true, typename T2 = void>
using FailIfV = typename v1::details::FailIfT<IntValue<V>, Flag, T2>::Type;


template <typename T1, typename T2>
constexpr bool compare_gt(T1&& first, T2&& second) {
    return first > second;
}

template <typename T1, typename T2>
constexpr bool compare_eq(T1&& first, T2&& second) {
    return first == second;
}

template <typename T1, typename T2>
constexpr bool compare_lt(T1&& first, T2&& second) {
    return first < second;
}

template <typename T1, typename T2>
constexpr bool compare_ge(T1&& first, T2&& second) {
    return first >= second;
}

template <typename T1, typename T2>
constexpr bool compare_le(T1&& first, T2&& second) {
    return first <= second;
}


template <typename T> class ValueCodec;
template <typename T> struct FieldFactory;

template <typename T>
class ValuePtrT1 {
    const T* addr_;
    size_t length_;
public:
    ValuePtrT1(): addr_(), length_() {}
    ValuePtrT1(const T* addr, size_t length): addr_(addr), length_(length) {}

    const T* addr() const {return addr_;}
    size_t length() const {return length_;}
};

template <typename T>
class ValuePtrT2 {
    const T* addr_;
    size_t offset_;
    size_t length_;
public:
    ValuePtrT2(): addr_(), offset_(0), length_() {}
    ValuePtrT2(const T* addr, size_t offset, size_t length): addr_(addr), offset_(offset), length_(length) {}

    const T* addr() const {return addr_;}
    size_t length() const {return length_;}
    size_t offset() const {return offset_;}
};


namespace {
    template <typename T> struct Void {
        using Type = void;
    };

    template <typename T>
    struct IsCompleteHelper {
        template <typename U>
        static auto test(U*)  -> std::integral_constant<bool, sizeof(U) == sizeof(U)>;
        static auto test(...) -> std::false_type;
        using Type = decltype(test((T*)0));
    };
}

template <typename T>
struct IsComplete : IsCompleteHelper<T>::Type {};


template <typename T>
struct HasValueCodec: HasValue<bool, IsComplete<ValueCodec<T>>::type::value> {};

template <typename T>
struct HasFieldFactory: HasValue<bool, IsComplete<FieldFactory<T>>::type::value> {};

template <typename T>
struct IsExternalizable: HasValue<bool, HasValueCodec<T>::Value || HasFieldFactory<T>::Value> {};

template <typename T> struct IOBufferAdapter;


enum class ByteOrder {
    BIG, LITTLE
};

enum class MemoryAccess {
    MMA_ALIGNED, MMA_UNALIGNED
};


template <typename T> struct TypeTag {};

#ifdef _MSC_VER

#define MEMORIA_V1_ALWAYS_INLINE 




uint32_t __inline __builtin_ctz(uint32_t value)
{
	DWORD trailing_zero = 0;

	if (_BitScanForward(&trailing_zero, value))
	{
		return trailing_zero;
	}
	else
	{
		return 32;
	}
	return 0;
}

uint32_t __inline __builtin_ctzll(uint64_t value)
{
	DWORD trailing_zero = 0;

	if (_BitScanForward64(&trailing_zero, value))
	{
		return trailing_zero;
	}
	else
	{
		return 64;
	}
	return 0;
}

uint32_t __inline __builtin_clz(uint32_t value)
{
	DWORD leading_zero = 0;

	if (_BitScanReverse(&leading_zero, value))
	{
		return 31 - leading_zero;
	}
	else
	{
		return 32;
	}
	return 0;
}

uint32_t __inline __builtin_clzll(uint64_t value)
{
	DWORD leading_zero = 0;

	if (_BitScanReverse64(&leading_zero, value))
	{
		return 63 - leading_zero;
	}
	else
	{
		return 64;
	}
	return 0;
}


#else 
#define MEMORIA_V1_ALWAYS_INLINE __attribute__((always_inline))
#endif


}}
