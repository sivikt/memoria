
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_CORE_PACKED_ALLOCATOR_HPP_
#define MEMORIA_CORE_PACKED_ALLOCATOR_HPP_

#include <memoria/core/types/types.hpp>
#include <memoria/core/types/type2type.hpp>

#include <memoria/core/packed2/packed_allocator_types.hpp>
#include <memoria/core/packed2/packed_fse_tree.hpp>

#include <memoria/core/tools/bitmap.hpp>
#include <memoria/core/tools/dump.hpp>

#include <type_traits>

namespace memoria {

using namespace std;

enum class PackedBlockType {
	RAW_MEMORY = 0, ALLOCATABLE = 1
};

class PackedAllocator: public PackedAllocatable {

	typedef PackedAllocatable													Base;
	typedef PackedAllocator														MyType;
	typedef PackedAllocator														Allocator;

	typedef PackedFSETreeTypes <Int, Int, Int>									LayoutTypes;

public:
	typedef PackedFSETree<LayoutTypes>											Layout;

	typedef UBigInt																Bitmap;

	static const UInt VERSION           										= 1;

private:
	Int block_size_;
	Int layout_size_;
	Int bitmap_size_;

	UByte buffer_[];

public:

	typedef typename MergeLists<
	            typename Base::FieldsList,

	            UIntValue<VERSION>,
	            decltype(block_size_),
	            decltype(layout_size_),
	            decltype(bitmap_size_)

	>::Result                                                                   FieldsList;

	PackedAllocator() {}

	bool is_allocatable(Int idx) const
	{
		const Bitmap* bmp = bitmap();
		return GetBit(bmp, idx);
	}

	Bitmap* bitmap() {
		return T2T<Bitmap*>(buffer_ + layout_size_);
	}

	const Bitmap* bitmap() const {
		return T2T<const Bitmap*>(buffer_ + layout_size_);
	}

	Int allocated() const {
		return element_offset(elements());
	}

	Int client_area() const {
		return block_size_ - sizeof(MyType) - layout_size_ - bitmap_size_;
	}

	Int free_space() const {
		 return client_area() - allocated();
	}

	Int elements() const {
		return layout_size_/4 - 1;
	}

	UByte* base() {
		return buffer_ + layout_size_ + bitmap_size_;
	}

	const UByte* base() const {
		return buffer_ + layout_size_ + bitmap_size_;
	}

	Int layout_size() const {
		return layout_size_;
	}

	Int bitmap_size() const {
		return bitmap_size_;
	}

	Int block_size() const {
		return block_size_;
	}

	void init(Int block_size, Int blocks)
	{
		block_size_ = roundDownBytesToAlignmentBlocks(block_size);

		Int layout_blocks = blocks + (blocks % 2 ? 1 : 2);

		layout_size_ = layout_blocks * sizeof(Int);

		memset(buffer_, 0, layout_size_);

		bitmap_size_ = roundUpBitsToAlignmentBlocks(layout_blocks);

		Bitmap* bitmap = this->bitmap();
		memset(bitmap, 0, bitmap_size_);
	}

	static Int block_size(Int client_area, Int blocks)
	{
		Int layout_blocks = blocks + (blocks % 2 ? 1 : 2);

		Int layout_size = layout_blocks * sizeof(Int);
		Int bitmap_size = roundUpBitsToAlignmentBlocks(blocks);

		return sizeof(MyType) + layout_size + bitmap_size + roundUpBytesToAlignmentBlocks(client_area);
	}

	Int computeElementOffset(const void* element) const
	{
		const UByte* base_ptr = base();
		const UByte* elt_ptr = T2T<const UByte*>(element);

		size_t diff = T2T<size_t>(elt_ptr - base_ptr);

		return diff;
	}

	Int resizeBlock(const void* element, Int new_size)
	{
		MEMORIA_ASSERT(new_size, >, 0);

		Int idx 		= findElement(element);

		Int allocation_size = roundUpBytesToAlignmentBlocks(new_size);

		Int size		= element_size(idx);
		Int delta 		= allocation_size - size;

		if (delta > 0)
		{
			if (delta > free_space())
			{
				enlarge(delta);
			}
		}

		moveElements(idx + 1, delta);

		return allocation_size;
	}

	Int* layout() {
		return T2T<Int*>(buffer_);
	}

	const Int* layout() const {
		return T2T<const Int*>(buffer_);
	}

	const Int& element_offset(Int idx) const
	{
		return *(T2T<const Int*>(buffer_) + idx);
	}

	Int element_size(Int idx) const
	{
		return element_offset(idx + 1) - element_offset(idx);
	}

	Int element_size(const void* element_ptr) const
	{
		Int idx = findElement(element_ptr);
		return element_size(idx);
	}


	Int findElement(const void* element_ptr) const
	{
		Int offset 	= computeElementOffset(element_ptr);

		for (Int c = 0; c < layout_size_ / 4; c++)
		{
			if (offset < element_offset(c))
			{
				return c - 1;
			}
		}

		throw Exception(MA_SRC, "Requested element is not found in thit allocator");
	}


	template <typename T>
	const T* get(Int idx) const
	{
		//MEMORIA_ASSERT(element_size(idx), >, 0);
		const T* addr = T2T<const T*>(base() + element_offset(idx));

		__builtin_prefetch(addr);
		return addr;
	}

	template <typename T>
	T* get(Int idx)
	{
		//MEMORIA_ASSERT(element_size(idx), >, 0);
		T* addr = T2T<T*>(base() + element_offset(idx));

		__builtin_prefetch(addr);

		return addr;
	}

	bool is_empty(int idx) const
	{
		return element_size(idx) == 0;
	}

	AllocationBlock describe(Int idx)
	{
		Int offset 	= element_offset(idx);
		Int size	= element_size(idx);

		return AllocationBlock(size, offset, base() + offset);
	}

	AllocationBlockConst describe(Int idx) const
	{
		Int offset 	= element_offset(idx);
		Int size	= element_size(idx);

		return AllocationBlockConst(size, offset, base() + offset);
	}

	template <typename T>
	T* allocate(Int idx, Int block_size)
	{
		static_assert(is_base_of<PackedAllocatable, T>::value, "Only derived classes of PackedAllocatable "
														"should be instantiated this way");

		AllocationBlock block = allocate(idx, block_size, PackedBlockType::ALLOCATABLE);

		T* object = block.cast<T>();

		object->init(block.size());

		return object;
	}

	template <typename T>
	T* allocate(Int idx)
	{
		static_assert(!is_base_of<PackedAllocatable, T>::value, "Only classes that are not derived from PackedAllocatable "
																"should be instantiated this way");

		AllocationBlock block = allocate(idx, sizeof(T), PackedBlockType::RAW_MEMORY);
		return block.cast<T>();
	}

	template <typename T>
	T* allocateArrayByLength(Int idx, Int length)
	{
		static_assert(!is_base_of<PackedAllocatable, T>::value, "Only classes that are not derived from PackedAllocatable "
				"should be instantiated this way");

		AllocationBlock block = allocate(idx, length, PackedBlockType::RAW_MEMORY);
		return block.cast<T>();
	}

	template <typename T>
	T* allocateArrayBySize(Int idx, Int size)
	{
		static_assert(!is_base_of<PackedAllocatable, T>::value, "Only classes that are not derived from PackedAllocatable "
				"should be instantiated this way");

		AllocationBlock block = allocate(idx, sizeof(T)*size, PackedBlockType::RAW_MEMORY);
		return block.cast<T>();
	}


	AllocationBlock allocate(Int idx, Int size, PackedBlockType type)
	{
		Int allocation_size = roundUpBytesToAlignmentBlocks(size);

		if (allocation_size > free_space())
		{
			enlarge(allocation_size - free_space());
		}

		moveElements(idx + 1, allocation_size);

		setBlockType(idx, type);

		Int offset = element_offset(idx);

		memset(base() + offset, 0, allocation_size);

		if (type == PackedBlockType::ALLOCATABLE)
		{
			PackedAllocatable* alc = T2T<PackedAllocatable*>(base() + offset);
			alc->setAllocatorOffset(this);
		}

		return AllocationBlock(allocation_size, offset, base() + offset);
	}

	void free(Int idx)
	{
		Int size 		= element_size(idx);
		moveElements(idx + 1, -size);
	}

	void clear(Int idx)
	{
		auto block = describe(idx);
		memset(block.ptr(), 0, block.size());
	}

	void setBlockType(Int idx, PackedBlockType type)
	{
		Bitmap* bitmap = this->bitmap();
		SetBit(bitmap, idx, type == PackedBlockType::ALLOCATABLE);
	}

	void dump(ostream& out = cout) const
	{
		out<<"PackedAllocator Layout:"<<endl;

		dumpLayout(out);

		out<<"PackedAllocator Block Types Bitmap:"<<endl;
		const Bitmap* bitmap = this->bitmap();

		dumpSymbols<Bitmap>(out, layout_size_/4, 1, [bitmap](Int idx){
			return GetBit(bitmap, idx);
		});
	}

	void dumpLayout(ostream& out = cout) const
	{
		dumpArray<Int>(out, layout_size_/4, [this](Int idx){
			return this->element_offset(idx);
		});
	}

	Int enlarge(Int delta)
	{
		return resize(block_size_ + roundUpBytesToAlignmentBlocks(delta));
	}


	Int resize(Int new_size)
	{
		if (allocator_offset() > 0)
		{
			Allocator* alloc = allocator();
			block_size_ = alloc->resizeBlock(this, new_size);
		}
		else if (new_size < block_size_)
		{
			if (new_size >= allocated() + (Int)sizeof(MyType) + layout_size_ + bitmap_size_)
			{
				block_size_ = new_size;
			}
			else {
				throw PackedOOMException(MA_SRC, SBuf()<<"Requested allocator size is too small: "
						<<new_size<<" bytes. Allocated = "<<allocated()<<" bytes.");
			}
		}
		else {
			throw PackedOOMException(MA_SRC, SBuf()<<"no space left in packed allocator: "
												   <<new_size<<" bytes. Allocated = "<<allocated()
												   <<" bytes, avaliable = "<<free_space()<<" bytes.");
		}

		return block_size_;
	}

	void forceResize(Int amount)
	{
		block_size_ += roundDownBytesToAlignmentBlocks(amount);
	}


	Int pack()
	{
		return resize(block_size_ - free_space());
	}


	void generateDataEvents(IPageDataEventHandler* handler) const
	{
		handler->startGroup("PACKED_ALLOCATOR");

		handler->value("ALLOCATOR",     &Base::allocator_offset());

		handler->value("BLOCK_SIZE",    &block_size_);
		handler->value("LAYOUT_SIZE",   &layout_size_);
		handler->value("BITMAP_SIZE",   &bitmap_size_);

		Int layout_size = layout_size_ / 4;

		handler->startGroup("LAYOUT", layout_size);

		for (Int idx = 0; idx < layout_size; idx++)
		{
			handler->value("OFFSET", &element_offset(idx));
		}

		handler->endGroup();

		handler->symbols("BITMAP", bitmap(), layout_size, 1);

		handler->endGroup();
	}

	void serialize(SerializationData& buf) const
	{
		FieldFactory<Int>::serialize(buf, allocator_offset_);
		FieldFactory<Int>::serialize(buf, block_size_);
		FieldFactory<Int>::serialize(buf, layout_size_);
		FieldFactory<Int>::serialize(buf, bitmap_size_);

		Int layout_size = layout_size_ / 4;

		FieldFactory<Int>::serialize(buf, layout(), layout_size);

		FieldFactory<Bitmap>::serialize(buf, bitmap(), bitmap_size_/sizeof(Bitmap));
	}

	void deserialize(DeserializationData& buf)
	{
		FieldFactory<Int>::deserialize(buf, allocator_offset_);
		FieldFactory<Int>::deserialize(buf, block_size_);
		FieldFactory<Int>::deserialize(buf, layout_size_);
		FieldFactory<Int>::deserialize(buf, bitmap_size_);

		Int layout_size = layout_size_ / 4;

		FieldFactory<Int>::deserialize(buf, layout(), layout_size);

		FieldFactory<Bitmap>::deserialize(buf, bitmap(), bitmap_size_/sizeof(Bitmap));
	}


private:

	Int& set_element_offset(Int idx)
	{
		return *(T2T<Int*>(buffer_) + idx);
	}


	void moveElementsUp(Int idx, int delta)
	{
		Int layout_size = layout_size_/4;

		if (idx < layout_size - 1)
		{
			AllocationBlock block = describe(idx);

			moveElementsUp(idx + 1, delta);

			moveElementData(idx, block, delta);
		}

		set_element_offset(idx) += delta;
	}

	void moveElementsDown(Int idx, int delta)
	{
		Int layout_size = layout_size_/4;

		if (idx < layout_size - 1)
		{
			AllocationBlock block = describe(idx);

			moveElementData(idx, block, delta);

			set_element_offset(idx) += delta;

			moveElementsDown(idx + 1, delta);
		}
		else {
			set_element_offset(idx) += delta;
		}
	}

	void moveElementData(Int idx, const AllocationBlock& block, Int delta)
	{
		if (block.size() > 0)
		{
			UByte* ptr = block.ptr();

			CopyByteBuffer(ptr, ptr + delta, block.size());

			if (is_allocatable(idx))
			{
				PackedAllocatable* element = T2T<PackedAllocatable*>(ptr + delta);
				element->setAllocatorOffset(this);
			}
		}
	}


	void moveElements(Int start_idx, Int delta)
	{
		if (delta > 0) {
			moveElementsUp(start_idx, delta);
		}
		else {
			moveElementsDown(start_idx, delta);
		}
	}

};

}


#endif