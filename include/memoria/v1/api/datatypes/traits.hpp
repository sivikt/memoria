
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

template <typename T> struct DataTypeTraits;

template <typename T, typename DataType>
struct PrimitiveDataTypeTraits
{
    using CxxType   = T;
    using InputView = T;
    using Ptr       = T*;

    using Parameters = TL<>;

    static constexpr size_t MemorySize        = sizeof(T);
    static constexpr bool IsParametrised      = false;
    static constexpr bool HasTypeConstructors = false;

    static void create_signature(SBuf& buf, DataType obj) {
        PrimitiveDataTypeName<DataType>::create_signature(buf, obj);
    }

    static void create_signature(SBuf& buf) {
        PrimitiveDataTypeName<DataType>::create_signature(buf, DataType());
    }
};

template <>
struct DataTypeTraits<TinyInt>: PrimitiveDataTypeTraits<int8_t, TinyInt> {};

template <>
struct DataTypeTraits<UTinyInt>: PrimitiveDataTypeTraits<uint8_t, UTinyInt> {};


template <>
struct DataTypeTraits<SmallInt>: PrimitiveDataTypeTraits<int16_t, SmallInt> {};

template <>
struct DataTypeTraits<USmallInt>: PrimitiveDataTypeTraits<uint16_t, USmallInt> {};


template <>
struct DataTypeTraits<Integer>: PrimitiveDataTypeTraits<int32_t, Integer> {};

template <>
struct DataTypeTraits<UInteger>: PrimitiveDataTypeTraits<uint32_t, UInteger> {};


template <>
struct DataTypeTraits<BigInt>: PrimitiveDataTypeTraits<int64_t, BigInt> {};

template <>
struct DataTypeTraits<UBigInt>: PrimitiveDataTypeTraits<uint64_t, UBigInt> {};


template <>
struct DataTypeTraits<Real>: PrimitiveDataTypeTraits<float, Real> {};

template <>
struct DataTypeTraits<Double>: PrimitiveDataTypeTraits<double, Double> {};

template <>
struct DataTypeTraits<Timestamp>: PrimitiveDataTypeTraits<int64_t, Timestamp> {};

template <>
struct DataTypeTraits<TSWithTimeZone>: PrimitiveDataTypeTraits<int64_t, TSWithTimeZone> {};

template <>
struct DataTypeTraits<Time>: PrimitiveDataTypeTraits<int32_t, Time> {};

template <>
struct DataTypeTraits<TimeWithTimeZone>: PrimitiveDataTypeTraits<int64_t, TimeWithTimeZone> {};

template <>
struct DataTypeTraits<Date>: PrimitiveDataTypeTraits<int64_t, Date> {};



template <>
struct DataTypeTraits<Decimal>
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
struct DataTypeTraits<BigDecimal>
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
struct DataTypeTraits<Varchar>
{
    using CxxType   = EmptyType;
    using InputView = EmptyType;
    using Ptr       = EmptyType*;

    using Parameters = TL<>;

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

template <typename T>
struct DataTypeTraits<Dynamic<T>>: DataTypeTraits<T>
{
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
struct DataTypeTraits<Multimap1<Key, Value>>
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