
// Copyright Victor Smirnov 2013+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memoria/v1/prototypes/bt_ss/btss_factory.hpp>

#include <memoria/v1/containers/vector/vctr_walkers.hpp>
#include <memoria/v1/containers/vector/vctr_tools.hpp>
#include <memoria/v1/containers/vector/vctr_names.hpp>

#include <memoria/v1/containers/vector/container/vctr_c_tools.hpp>
#include <memoria/v1/containers/vector/container/vctr_c_insert.hpp>
#include <memoria/v1/containers/vector/container/vctr_c_remove.hpp>
#include <memoria/v1/containers/vector/container/vctr_c_api.hpp>
#include <memoria/v1/containers/vector/container/vctr_c_find.hpp>

#include <memoria/v1/containers/vector/vctr_iterator.hpp>
#include <memoria/v1/containers/vector/iterator/vctr_i_api.hpp>

#include <memoria/v1/containers/vector/vctr_names.hpp>

#include <memoria/v1/core/packed/array/packed_fse_array.hpp>
#include <memoria/v1/core/packed/array/packed_vle_dense_array.hpp>
#include <memoria/v1/core/packed/misc/packed_sized_struct.hpp>

namespace memoria {



template <typename Profile, typename Value_>
struct VectorBTTypesBase: public BTTypes<Profile, memoria::BTSingleStream> {

    using Base = BTTypes<Profile, memoria::BTSingleStream>;

    using Value = Value_;
    using Entry = Value_;


    using CommonContainerPartsList = MergeLists<
            typename Base::CommonContainerPartsList,

            mvector::CtrToolsName,
            mvector::CtrInsertName,
            mvector::CtrRemoveName,
            mvector::CtrFindName,
            mvector::CtrApiName
    >;

    using IteratorPartsList = MergeLists<
            typename Base::IteratorPartsList,
            mvector::ItrApiName
    >;
};





template <typename Profile, typename Value>
struct BTTypes<Profile, memoria::Vector<Value> >: public VectorBTTypesBase<Profile, Value> {

    static_assert(
            IsExternalizable<Value>::Value ,
            "Value type must have either ValueCodec or FieldFactory defined"
    );


    using LeafValueStruct = typename mvector::VectorValueStructTF<Value, HasFieldFactory<Value>::Value>::Type;


    using StreamDescriptors = TL<StreamTF<
        TL<
            StreamSize,
            LeafValueStruct
        >,
        DefaultBranchStructTF,
        TL<TL<>, TL<>>
    >>;
};



template <Granularity Gr> struct CodecClassTF;

template <>
struct CodecClassTF<Granularity::Byte> {
    template <typename V>
    using Type = UByteI7Codec<V>;
};


template <>
struct CodecClassTF<Granularity::Bit> {
    template <typename V>
    using Type = UBigIntEliasCodec<V>;
};


template <typename Profile, Granularity Gr, typename Value_>
struct BTTypes<Profile, memoria::Vector<VLen<Gr, Value_>> >: public BTTypes<Profile, memoria::BTSingleStream> {

    typedef BTTypes<Profile, memoria::BTSingleStream>                           Base;

    typedef Value_                                                              Value;

    using VectorStreamTF = StreamTF<
        TL<TL<
            StreamSize,
            PkdVDArrayT<Value, 1, CodecClassTF<Gr>::template Type>
        >>,
        FSEBranchStructTF,
        TL<TL<TL<>, TL<>>>
    >;


    typedef TypeList<
                VectorStreamTF
    >                                                                           StreamDescriptors;

    using Entry = Value;

    using CommonContainerPartsList = MergeLists<
            typename Base::CommonContainerPartsList,

            mvector::CtrToolsName,
            mvector::CtrInsertName,
            mvector::CtrRemoveName,
            mvector::CtrFindName,
            mvector::CtrApiName
    >;

    using IteratorPartsList = MergeLists<
            typename Base::IteratorPartsList,
            mvector::ItrApiName
    >;
};








template <typename Profile, typename Value, typename T>
class CtrTF<Profile, memoria::Vector<Value>, T>: public CtrTF<Profile, memoria::BTSingleStream, T> {

    using Base = CtrTF<Profile, memoria::BTSingleStream, T>;
public:

    struct Types: Base::Types
    {
        typedef Vector2CtrTypes<Types>                                          CtrTypes;
        typedef Vector2IterTypes<Types>                                         IterTypes;

        typedef PageUpdateManager<CtrTypes>                                     PageUpdateMgr;
    };


    typedef typename Types::CtrTypes                                            CtrTypes;
    typedef Ctr<CtrTypes>                                                       Type;

};




}
