
// Copyright Victor Smirnov 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memoria/v1/core/tools/md5.hpp>
#include <memoria/v1/core/types/type2type.hpp>

#include <string.h>
#include <tuple>

namespace memoria {

inline BigInt PtrToLong(const void *ptr) {
    return T2T<BigInt>(ptr);
}

template <typename T> struct FieldFactory;

template <typename Type>
struct CompositeFieldFactory {

    static void serialize(SerializationData& data, const Type& field)
    {
        field.serialize(data);
    }

    static void serialize(SerializationData& data, const Type* field, Int size)
    {
        for (Int c = 0; c < size; c++)
        {
            field[c].serialize(data);
        }
    }


    static void deserialize(DeserializationData& data, Type& field)
    {
        field.deserialize(data);
    }

    static void deserialize(DeserializationData& data, Type* field, Int size)
    {
        for (Int c = 0; c < size; c++)
        {
            field[c].deserialize(data);
        }
    }
};

template <typename Type>
class BitField{};

template <typename Type>
struct FieldFactory<BitField<Type> > {

    typedef BitField<Type> BFType;

    static void serialize(SerializationData& data, const Type& field)
    {
        field.template serialize<FieldFactory>(data);
    }

    static void deserialize(DeserializationData& data, Type& field)
    {
        field.template deserialize<FieldFactory>(data);
    }
};

template <>
struct FieldFactory<EmptyValue> {
    static void serialize(SerializationData& data, const EmptyValue& field, Int count = 1) {}
    static void serialize(SerializationData& data, const EmptyValue* field, Int count = 1) {}

    static void deserialize(DeserializationData& data, EmptyValue& field, Int count = 1) {}
    static void deserialize(DeserializationData& data, EmptyValue* field, Int count = 1) {}
};



#define MEMORIA_TYPED_FIELD(Type)                                               \
template <> struct FieldFactory<Type> {                                         \
                                                                                \
    static void serialize(SerializationData& data, const Type& field) {         \
        memmove(data.buf, &field, sizeof(Type));                                \
        data.buf += sizeof(Type);                                               \
        data.total += sizeof(Type);                                             \
    }                                                                           \
    static void deserialize(DeserializationData& data, Type& field) {           \
        memmove(&field, data.buf, sizeof(Type));                                \
        data.buf += sizeof(Type);                                               \
    }                                                                           \
                                                                                \
    static void serialize(SerializationData& data, const Type& field, Int count) {\
        memmove(data.buf, &field, count*sizeof(Type));                          \
        data.buf += count * sizeof(Type);                                       \
        data.total += count * sizeof(Type);                                     \
    }                                                                           \
    static void deserialize(DeserializationData& data, Type& field, Int count) {\
        memmove(&field, data.buf, count*sizeof(Type));                          \
        data.buf += count*sizeof(Type);                                         \
    }                                                                           \
    static void serialize(SerializationData& data, const Type* field, Int count) {\
        memmove(data.buf, field, count*sizeof(Type));                           \
        data.buf += count * sizeof(Type);                                       \
        data.total += count * sizeof(Type);                                     \
    }                                                                           \
    static void deserialize(DeserializationData& data, Type* field, Int count) {\
        memmove(field, data.buf, count*sizeof(Type));                           \
        data.buf += count*sizeof(Type);                                         \
    }                                                                           \
}


template <> struct FieldFactory<UBigInt> {

    using Type = UBigInt;

    static void serialize(SerializationData& data, const Type& field) {
        memmove(data.buf, &field, sizeof(Type));
        data.buf += sizeof(Type);
        data.total += sizeof(Type);
    }
    static void deserialize(DeserializationData& data, Type& field) {
        memmove(&field, data.buf, sizeof(Type));
        data.buf += sizeof(Type);
    }

    static void serialize(SerializationData& data, const Type& field, Int count) {
        memmove(data.buf, &field, count*sizeof(Type));
        data.buf += count * sizeof(Type);
        data.total += count * sizeof(Type);
    }
    static void deserialize(DeserializationData& data, Type& field, Int count) {
        memmove(&field, data.buf, count*sizeof(Type));
        data.buf += count*sizeof(Type);
    }
    static void serialize(SerializationData& data, const Type* field, Int count) {
        memmove(data.buf, field, count*sizeof(Type));
        data.buf += count * sizeof(Type);
        data.total += count * sizeof(Type);
    }
    static void deserialize(DeserializationData& data, Type* field, Int count) {
        memmove(field, data.buf, count*sizeof(Type));
        data.buf += count*sizeof(Type);
    }
};


MEMORIA_TYPED_FIELD(Char);
MEMORIA_TYPED_FIELD(Byte);
MEMORIA_TYPED_FIELD(Short);
MEMORIA_TYPED_FIELD(Int);
MEMORIA_TYPED_FIELD(BigInt);
MEMORIA_TYPED_FIELD(UByte);
MEMORIA_TYPED_FIELD(UShort);
MEMORIA_TYPED_FIELD(UInt);
MEMORIA_TYPED_FIELD(float);
MEMORIA_TYPED_FIELD(double);
MEMORIA_TYPED_FIELD(bool);


namespace internal {

template <typename Tuple, Int Idx = std::tuple_size<Tuple>::value - 1>
struct TupleFactoryHelper {

    using CurrentType = typename std::tuple_element<Idx, Tuple>::type;

    static void serialize(SerializationData& data, const Tuple& field)
    {
        FieldFactory<CurrentType>::serialize(data, std::get<Idx>(field));

        TupleFactoryHelper<Tuple, Idx - 1>::serialize(data, field);
    }

    static void deserialize(DeserializationData& data, Tuple& field)
    {
        FieldFactory<CurrentType>::deserialize(data, std::get<Idx>(field));

        TupleFactoryHelper<Tuple, Idx - 1>::deserialize(data, field);
    }
};

template <typename Tuple>
struct TupleFactoryHelper<Tuple, -1> {
    static void serialize(SerializationData& data, const Tuple& field) {}
    static void deserialize(DeserializationData& data, Tuple& field) {}
};

}


template <typename... Types>
struct FieldFactory<std::tuple<Types...> > {

    using Type = std::tuple<Types...>;

    static void serialize(SerializationData& data, const Type& field)
    {
        memoria::internal::TupleFactoryHelper<Type>::serialize(data, field);
    }

    static void serialize(SerializationData& data, const Type* field, Int size)
    {
        for (Int c = 0; c < size; c++)
        {
            memoria::internal::TupleFactoryHelper<Type>::serialize(data, field[c]);
        }
    }

    static void deserialize(DeserializationData& data, Type& field)
    {
        memoria::internal::TupleFactoryHelper<Type>::deserialize(data, field);
    }

    static void deserialize(DeserializationData& data, Type* field, Int size)
    {
        for (Int c = 0; c < size; c++)
        {
            memoria::internal::TupleFactoryHelper<Type>::deserialize(data, field[c]);
        }
    }
};



#undef MEMORIA_TYPED_FIELD


}
