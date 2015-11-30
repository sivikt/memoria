
// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef MEMORIA_CORE_PACKED_VLE_TOOLS_HPP_
#define MEMORIA_CORE_PACKED_VLE_TOOLS_HPP_

#include <memoria/core/types/types.hpp>


namespace memoria {

template <typename Codec>
struct PkdVLETreeShapeProvider {
	static constexpr Int BitsPerElement = Codec::ElementSize;
	static constexpr Int BlockSize = 128;// bytes

	static constexpr Int BranchingFactor = PackedTreeBranchingFactor;
	static constexpr Int ValuesPerBranch = BlockSize * 8 / BitsPerElement;
};

template <
	typename IndexValueT,
	Int kBlocks,
	template <typename> class CodecT,
	typename ValueT = BigInt,
	Int kBranchingFactor = PkdVLETreeShapeProvider<CodecT<ValueT>>::BranchingFactor,
	Int kValuesPerBranch = PkdVLETreeShapeProvider<CodecT<ValueT>>::ValuesPerBranch
>
struct PkdVLETreeTypes {
	using IndexValue 	= IndexValueT;
	using Value 		= ValueT;

	template <typename T>
	using Codec = CodecT<T>;

	static constexpr Int Blocks = kBlocks;
	static constexpr Int BranchingFactor = kBranchingFactor;
	static constexpr Int ValuesPerBranch = kValuesPerBranch;
};



}


#endif
