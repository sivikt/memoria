
// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_BTTL_TEST_FACTORY1_HPP_
#define MEMORIA_TESTS_BTTL_TEST_FACTORY1_HPP_

#include <memoria/memoria.hpp>

#include <memoria/tools/profile_tests.hpp>
#include <memoria/tools/tools.hpp>

#include <memoria/prototypes/bt_tl/bttl_factory.hpp>
#include <memoria/prototypes/bt_tl/tools/bttl_random_gen.hpp>

#include <memoria/core/types/typehash.hpp>


#include "bttl_test_tools.hpp"



#include <functional>

namespace memoria {


template <Int Levels, PackedSizeType SizeType>
class BTTLTestCtr {};


template <
    typename Profile,
    Int Levels,
	PackedSizeType SizeType
>
struct BTTLTestTypesBase: public BTTypes<Profile, BTTreeLayout> {

    using Base = BTTypes<Profile, BTTreeLayout>;

    using ValueType = Byte;


    using Key 	= BigInt;
    using Value	= Byte;

    using CtrSizeT = BigInt;


    template <Int Indexes>
    struct Stream1VariableTF {
        using NonLeafType 	= PkdFTree<Packed2TreeTypes<Key, Key, Indexes + 2>>;
        using LeafType 		= TL<TL<
        	PkdVTree<Packed2TreeTypes<Key, Key, Indexes, UByteI7Codec>>,
			PkdVTree<Packed2TreeTypes<CtrSizeT, CtrSizeT, 1, UByteI7Codec>>
        >>;

        using IdxRangeList 	= TL<TL<TL<IndexRange<0, Indexes>>, TL<IndexRange<0, 1>>>>;
    };


    template <Int Indexes>
    struct Stream1FixedTF {
        using NonLeafType 	= PkdFTree<Packed2TreeTypes<Key, Key, Indexes + 2>>;
        using LeafType 		= TL<TL<
        	PkdFTree<Packed2TreeTypes<Key, Key, Indexes>>,
			PkdFTree<Packed2TreeTypes<CtrSizeT, CtrSizeT, 1>>
        >>;

        using IdxRangeList 	= TL<TL<TL<IndexRange<0, Indexes>>, TL<IndexRange<0, 1>>>>;
    };

    struct StreamNVariableTF {
        using NonLeafType 	= PkdFTree<Packed2TreeTypes<Key, Key, 2>>;
        using LeafType 		= TL<TL<
			PkdVTree<Packed2TreeTypes<CtrSizeT, CtrSizeT, 1, UByteI7Codec>>
        >>;

        using IdxRangeList 	= TL<TL<TL<IndexRange<0, 1>>>>;
    };

    struct StreamNFixedTF {
        using NonLeafType 	= PkdFTree<Packed2TreeTypes<Key, Key, 2>>;
        using LeafType 		= TL<TL<
        	PkdFTree<Packed2TreeTypes<CtrSizeT, CtrSizeT, 1>>
        >>;

        using IdxRangeList 	= TL<TL<TL<IndexRange<0, 1>>>>; //TL<IndexRange<0, Indexes>>,
    };

    struct DataStreamTF {
    	using NonLeafType 	= PkdFTree<Packed2TreeTypes<CtrSizeT, CtrSizeT, 1>>;
    	using LeafType 		= TL<PackedFSEArray<PackedFSEArrayTypes<Value>>>;

    	using IdxRangeList 	= TL<TL<>>;
    };


    using StreamDescriptors = typename IfThenElse<
    		SizeType == PackedSizeType::FIXED,
			MergeLists<
				Stream1FixedTF<1>,
				typename MakeList<StreamNFixedTF, Levels - 2>::Type,
				DataStreamTF
			>,
			MergeLists<
				Stream1VariableTF<1>,
				typename MakeList<StreamNVariableTF, Levels - 2>::Type,
				DataStreamTF
			>
    >::Result;

    using Metadata = BalancedTreeMetadata<
            typename Base::ID,
            ListSize<StreamDescriptors>::Value
    >;


    using CommonContainerPartsList = MergeLists<
                typename Base::CommonContainerPartsList,
                memoria::table::CtrApiName
    >;

    using IteratorPartsList = MergeLists<
                typename Base::IteratorPartsList,
                memoria::table::ItrMiscName
    >;
};







template <
    typename Profile,
    Int Levels,
	PackedSizeType SizeType
>
struct BTTypes<Profile, BTTLTestCtr<Levels, SizeType>>: public BTTLTestTypesBase<Profile, Levels, SizeType>
{
};


template <typename Profile, Int Levels, PackedSizeType SizeType, typename T>
class CtrTF<Profile, BTTLTestCtr<Levels, SizeType>, T>: public CtrTF<Profile, memoria::BTTreeLayout, T> {
    using Base = CtrTF<Profile, memoria::BTTreeLayout, T>;
public:

//    struct Types: Base::Types
//    {
//    	using CtrTypes 			= TableCtrTypes<Types>;
//        using IterTypes 		= TableIterTypes<Types>;
//
//        using PageUpdateMgr 	= PageUpdateManager<CtrTypes>;
//    };
//
//    using CtrTypes 	= typename Types::CtrTypes;
//    using Type 		= Ctr<CtrTypes>;
};


template <Int Level, PackedSizeType SizeType>
struct TypeHash<BTTLTestCtr<Level, SizeType>>:   UIntValue<
    HashHelper<3001, Level, (Int)SizeType>::Value
> {};


}


#endif
