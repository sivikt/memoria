
// Copyright 2019 Victor Smirnov
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

#include <memoria/v1/api/datatypes/core.hpp>
#include <memoria/v1/api/datatypes/type_signature.hpp>
#include <memoria/v1/core/strings/string_buffer.hpp>
#include <memoria/v1/core/tools/type_name.hpp>

#include <memoria/v1/api/datatypes/varbinaries.hpp>
#include <memoria/v1/api/datatypes/varchars.hpp>

namespace memoria {
namespace v1 {

template <typename T> struct PrimitiveDataTypeName;

#define MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(TypeName, TypeStr)  \
template <>                                             \
struct PrimitiveDataTypeName<TypeName> {                \
    static void create_signature(SBuf& buf, TypeName) { \
        buf << MMA1_TOSTRING(TypeStr);                  \
    }                                                   \
}

MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(TinyInt, TinyInt);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(SmallInt, SmallInt);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(Integer, Integer);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(BigInt, BigInt);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(Real, Real);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(Double, Double);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(Timestamp, Timestamp);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(TSWithTimeZone, Timestamp With Timezone);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(Time, Time);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(TimeWithTimeZone, Time With Timezone);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(Date, Date);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(uint8_t, UByte);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(int64_t, Int64);
MMA1_DECLARE_PRIMITIVE_DATATYPE_NAME(uint64_t, UInt64);


template <typename T> struct DataTypeTraits {
    static constexpr bool isDataType = false;
};

template <typename T>
using DTViewType = typename DataTypeTraits<T>::ViewType;

template <typename T>
using DTValueType = typename DataTypeTraits<T>::ValueType;

template <typename T> struct DataTypeTraitsBase {
    static constexpr bool isDataType = true;
    static constexpr bool isFixedSize = false;
};


template <typename T, typename DataType>
struct FixedSizeDataTypeTraits: DataTypeTraitsBase<DataType>
{
    using ViewType  = T;
    using ConstViewType = T;
    using ValueType = T;

    using Parameters = TL<>;

    static constexpr size_t MemorySize        = sizeof(T);
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = false;
    static constexpr bool isFixedSize         = true;

    static void create_signature(SBuf& buf, DataType obj) {
        PrimitiveDataTypeName<DataType>::create_signature(buf, obj);
    }

    static void create_signature(SBuf& buf) {
        PrimitiveDataTypeName<DataType>::create_signature(buf, DataType());
    }
};

template <>
struct DataTypeTraits<TinyInt>: FixedSizeDataTypeTraits<int8_t, TinyInt> {};

template <>
struct DataTypeTraits<UTinyInt>: FixedSizeDataTypeTraits<uint8_t, UTinyInt> {};

template <>
struct DataTypeTraits<uint8_t>: FixedSizeDataTypeTraits<uint8_t, uint8_t> {};


template <>
struct DataTypeTraits<SmallInt>: FixedSizeDataTypeTraits<int16_t, SmallInt> {};

template <>
struct DataTypeTraits<USmallInt>: FixedSizeDataTypeTraits<uint16_t, USmallInt> {};


template <>
struct DataTypeTraits<Integer>: FixedSizeDataTypeTraits<int32_t, Integer> {};

template <>
struct DataTypeTraits<UInteger>: FixedSizeDataTypeTraits<uint32_t, UInteger> {};


template <>
struct DataTypeTraits<BigInt>: FixedSizeDataTypeTraits<int64_t, BigInt> {};

template <>
struct DataTypeTraits<int64_t>: FixedSizeDataTypeTraits<int64_t, int64_t> {};

template <>
struct DataTypeTraits<UBigInt>: FixedSizeDataTypeTraits<uint64_t, UBigInt> {};

template <>
struct DataTypeTraits<uint64_t>: FixedSizeDataTypeTraits<uint64_t, uint64_t> {};

template <>
struct DataTypeTraits<Real>: FixedSizeDataTypeTraits<float, Real> {};

template <>
struct DataTypeTraits<Double>: FixedSizeDataTypeTraits<double, Double> {};

template <>
struct DataTypeTraits<Timestamp>: FixedSizeDataTypeTraits<int64_t, Timestamp> {};

template <>
struct DataTypeTraits<TSWithTimeZone>: FixedSizeDataTypeTraits<int64_t, TSWithTimeZone> {};

template <>
struct DataTypeTraits<Time>: FixedSizeDataTypeTraits<int32_t, Time> {};

template <>
struct DataTypeTraits<TimeWithTimeZone>: FixedSizeDataTypeTraits<int64_t, TimeWithTimeZone> {};

template <>
struct DataTypeTraits<Date>: FixedSizeDataTypeTraits<int64_t, Date> {};

template <>
struct DataTypeTraits<U8String>: DataTypeTraitsBase<U8String> {

    using ViewType      = VarcharView;
    using ConstViewType = VarcharView;
    using ValueType     = U8String;

    using Parameters = TL<>;

    static constexpr size_t MemorySize        = sizeof(EmptyType);
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = false;
    static constexpr bool isFixedSize         = false;

    static void create_signature(SBuf& buf, const U8String& obj) {
        buf << "U8String";
    }

    static void create_signature(SBuf& buf) {
        buf << "U8String";
    }
};


template <>
struct DataTypeTraits<Decimal>: DataTypeTraitsBase<Decimal>
{
    using CxxType   = EmptyType;
    using InputView = EmptyType;
    using Ptr       = EmptyType*;

    using Parameters = TL<>;

    static constexpr size_t MemorySize        = sizeof(EmptyType);
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = true;

    static void create_signature(SBuf& buf, const Decimal& obj)
    {
        buf << "Decimal";

        if (obj.is_default())
        {
            buf << "()";
        }
        else {
            buf << "(" << obj.precision() << ", " << obj.scale() << ")";
        }
    }

    static void create_signature(SBuf& buf)
    {
        buf << "Decimal";
    }
};


template <>
struct DataTypeTraits<BigDecimal>: DataTypeTraitsBase<BigDecimal>
{
    using CxxType   = EmptyType;
    using InputView = EmptyType;
    using Ptr       = EmptyType*;

    using Parameters = TL<>;

    static constexpr bool isDataType          = true;
    static constexpr size_t MemorySize        = sizeof(EmptyType);
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = true;

    static void create_signature(SBuf& buf, const Decimal& obj)
    {
        buf << "BigDecimal";

        if (obj.is_default())
        {
            buf << "()";
        }
        else {
            buf << "(" << obj.precision() << ", " << obj.scale() << ")";
        }
    }

    static void create_signature(SBuf& buf)
    {
        buf << "BigDecimal";
    }
};


template <>
struct DataTypeTraits<Varchar>: DataTypeTraitsBase<Varchar>
{
    using ViewType      = VarcharView;
    using ConstViewType = VarcharView;
    using ValueType     = U8String;

    using Parameters = TL<>;

    static constexpr bool isDataType          = true;
    static constexpr size_t MemorySize        = sizeof(EmptyType);
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = false;

    static void create_signature(SBuf& buf, const Varchar& obj)
    {
        buf << "Varchar";
    }

    static void create_signature(SBuf& buf)
    {
        buf << "Varchar";
    }
};



template <>
struct DataTypeTraits<Varbinary>: DataTypeTraitsBase<Varbinary>
{
    using AtomType      = uint8_t;
    using ViewType      = VarbinaryView<AtomType>;
    using ConstViewType = VarbinaryView<const AtomType>;
    using ValueType     = std::vector<AtomType>;

    using Parameters = TL<>;

    static constexpr bool isDataType          = true;
    static constexpr size_t MemorySize        = sizeof(EmptyType);
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = false;

    static void create_signature(SBuf& buf, const Varchar& obj)
    {
        buf << "Varbinary";
    }

    static void create_signature(SBuf& buf)
    {
        buf << "Varbinary";
    }
};

template <typename T>
struct DataTypeTraits<Dynamic<T>>: DataTypeTraits<T>
{
    static constexpr bool isDataType          = true;
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = false;

    using Parameters = TL<>;

    static void create_signature(SBuf& buf, T obj)
    {
        buf << "Dynamic ";
        DataTypeTraits<T>::create_signature(buf);
    }

    static void create_signature(SBuf& buf) {
        buf << "Dynamic ";
        DataTypeTraits<T>::create_signature(buf);
    }
};


template <typename Key, typename Value>
struct DataTypeTraits<Multimap1<Key, Value>>: DataTypeTraitsBase<Multimap1<Key, Value>>
{
    using CxxType   = EmptyType;
    using InputView = EmptyType;
    using Ptr       = EmptyType*;

    using Parameters = TL<Key, Value>;

    static constexpr size_t MemorySize        = sizeof(EmptyType);
    static constexpr bool IsParametrised      = true;
    static constexpr bool HasTypeConstructors = false;

    static void create_signature(SBuf& buf, const Multimap1<Key, Value>& obj)
    {
        buf << "Multimap1<";

        DataTypeTraits<Key>::create_signature(buf, obj.key());
        buf << ", ";
        DataTypeTraits<Value>::create_signature(buf, obj.value());

        buf << ">";
    }

    static void create_signature(SBuf& buf)
    {
        buf << "Multimap1<";

        DataTypeTraits<Key>::create_signature(buf);
        buf << ", ";
        DataTypeTraits<Value>::create_signature(buf);

        buf << ">";
    }
};


template<typename T>
TypeSignature make_datatype_signature(T obj)
{
    SBuf buf;
    DataTypeTraits<T>::create_signature(buf, obj);
    return TypeSignature(buf.str());
}

template<typename T>
TypeSignature make_datatype_signature()
{
    SBuf buf;
    DataTypeTraits<T>::create_signature(buf);
    return TypeSignature(buf.str());
}

}}
