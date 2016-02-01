
// Copyright Victor Smirnov 2016+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef MEMORIA_CORE_PACKED_FSE_MAX_TREE_BASE_HPP_
#define MEMORIA_CORE_PACKED_FSE_MAX_TREE_BASE_HPP_

#include <memoria/core/packed/tree/fse_max/packed_fse_max_tree_base_base.hpp>


namespace memoria {

template <typename ValueT, Int kBranchingFactor, Int kValuesPerBranch> class PkdFMTreeBase;

class PkdFMTreeMetadata {
	Int size_;
	Int index_size_;
	Int max_size_;
public:
	PkdFMTreeMetadata() = default;

	Int& size() {return size_;}
	const Int& size() const {return size_;}

	Int& index_size() {return index_size_;}
	const Int& index_size() const {return index_size_;}

	Int& max_size() {return max_size_;}
	const Int& max_size() const {return max_size_;}

	Int capacity() const {
		return max_size_ - size_;
	}

	template <typename, Int, Int, Int, typename> friend class PkdFMTreeBaseBase;
	template <typename, Int, Int> friend class PkdFMTreeBase;
};



template <typename ValueT, Int kBranchingFactor, Int kValuesPerBranch>
class PkdFMTreeBase: public PkdFMTreeBaseBase<ValueT, kBranchingFactor, kValuesPerBranch, 2, PkdFMTreeMetadata> {

	using Base 		= PkdFMTreeBaseBase<ValueT, kBranchingFactor, kValuesPerBranch, 2, PkdFMTreeMetadata>;
	using MyType 	= PkdFMTreeBase<ValueT, kBranchingFactor, kValuesPerBranch>;

public:
    static constexpr UInt VERSION = 1;


    using Value 		= ValueT;
    using IndexValue 	= ValueT;

    using Metadata		= typename Base::Metadata;
    using TreeLayout	= typename Base::template IndexedTreeLayout<IndexValue>;


    static const Int BranchingFactor        = kBranchingFactor;
    static const Int ValuesPerBranch        = kValuesPerBranch;

    static const bool FixedSizeElement      = true;

    static constexpr Int ValuesPerBranchMask 	= ValuesPerBranch - 1;
    static constexpr Int BranchingFactorMask 	= BranchingFactor - 1;

    static constexpr Int ValuesPerBranchLog2 	= Log2(ValuesPerBranch) - 1;
    static constexpr Int BranchingFactorLog2 	= Log2(BranchingFactor) - 1;

    static constexpr Int SegmentsPerBlock = 2;

    static constexpr Int METADATA = 0;


    struct InitFn {
    	Int blocks_;

    	InitFn(Int blocks): blocks_(blocks) {}

    	Int block_size(Int items_number) const {
    		return MyType::block_size(blocks_, items_number);
    	}

    	Int max_elements(Int block_size)
    	{
    		return block_size;
    	}
    };

public:

    PkdFMTreeBase() = default;

    using FieldsList = MergeLists<
                typename Base::FieldsList,
				decltype(Metadata::max_size_),
                ConstValue<UInt, VERSION>,
				Value
    >;


    void init_tl(Int data_block_size, Int blocks)
    {
    	Base::init(data_block_size, blocks * SegmentsPerBlock + 1);

    	Metadata* meta = this->template allocate<Metadata>(METADATA);

    	Int max_size        = 0;

    	meta->size()        = 0;
    	meta->max_size()   	= max_size;
    	meta->index_size()  = MyType::index_size(max_size);

    	for (Int block = 0; block < blocks; block++)
    	{
    		this->template allocateArrayBySize<IndexValue>(block * SegmentsPerBlock + 1, meta->index_size());
    		this->template allocateArrayBySize<Value>(block * SegmentsPerBlock + 2, max_size);
    	}
    }



    static Int block_size(Int blocks, Int capacity)
    {
    	Int metadata_length = Base::roundUpBytesToAlignmentBlocks(sizeof(Metadata));

    	Int index_size      = MyType::index_size(capacity);
    	Int index_length    = Base::roundUpBytesToAlignmentBlocks(index_size * sizeof(IndexValue));

    	Int values_length   = Base::roundUpBytesToAlignmentBlocks(capacity * sizeof(Value));

    	return Base::block_size(metadata_length + (index_length + values_length) * blocks, blocks * SegmentsPerBlock + 1);
    }

    Int block_size() const
    {
    	return Base::block_size();
    }

    static Int index_size(Int capacity)
    {
    	TreeLayout layout;
    	Base::compute_tree_layout(capacity, layout);
    	return layout.index_size;
    }

    static Int tree_size(Int blocks, Int block_size)
    {
    	return block_size >= (Int)sizeof(Value) ? FindTotalElementsNumber2(block_size, InitFn(blocks)) : 0;
    }


    Metadata* metadata() {
    	return this->template get<Metadata>(METADATA);
    }
    const Metadata* metadata() const {
    	return this->template get<Metadata>(METADATA);
    }

    IndexValue* index(Int block) {
    	return Base::template index<IndexValue, 0>(block);
    }

    const IndexValue* index(Int block) const {
    	return Base::template index<IndexValue, 0>(block);
    }


    bool has_index() const {
    	return true;
    }


    Value* values(Int block) {
    	return this->template get<Value>(block * SegmentsPerBlock + 2);
    }
    const Value* values(Int block) const {
    	return this->template get<Value>(block * SegmentsPerBlock + 2);
    }

    Value& value(Int block, Int idx) {
    	return values(block)[idx];
    }

    const Value& value(Int block, Int idx) const {
    	return values(block)[idx];
    }


    struct FindGEWalker {
    	IndexValue target_;
    	Int idx_;
    public:
    	FindGEWalker(IndexValue target): target_(target) {}

    	template <typename T>
    	bool compare(T value)
    	{
    		return value >= target_;
    	}

    	void next() {}

    	Int& idx() {return idx_;}
    	const Int& idx() const {return idx_;}

    	FindGEWalker& idx(Int value) {
    		idx_ = value;
    		return *this;
    	}
    };

    struct FindGTWalker {
    	IndexValue target_;

    	Int idx_;
    public:
    	FindGTWalker(IndexValue target): target_(target) {}

    	template <typename T>
    	bool compare(T value)
    	{
    		return value > target_;
    	}

    	void next() {}

    	Int& idx() {return idx_;}
    	const Int& idx() const {return idx_;}

    	FindGTWalker& idx(Int value) {
    		idx_ = value;
    		return *this;
    	}
    };



    auto find_ge(Int block, IndexValue value) const
    {
    	return find(block, FindGEWalker(value));
    }

    auto find_gt(Int block, IndexValue value) const
    {
    	return find(block, FindGTWalker(value));
    }

    template <typename Walker>
    auto find(Int block, Walker&& walker) const
    {
    	auto metadata = this->metadata();
    	auto values = this->values(block);

    	Int size = metadata->size();

    	if (this->element_size(block * SegmentsPerBlock + 1) == 0)
    	{
    		for (Int c = 0; c < size; c++)
    		{
    			if (walker.compare(values[c]))
    			{
    				return walker.idx(c);
    			}
    			else {
    				walker.next();
    			}
    		}

    		return walker.idx(size);
    	}
    	else {
    		TreeLayout data;

    		this->compute_tree_layout(metadata, data);

    		data.indexes = this->index(block);

    		Int idx = this->find_index(data, walker);

    		if (idx >= 0)
    		{
    			idx <<= ValuesPerBranchLog2;

    			for (Int c = idx; c < size; c++)
    			{
    				if (walker.compare(values[c]))
    				{
    					return walker.idx(c);
    				}
    				else {
    					walker.next();
    				}
    			}

    			return walker.idx(size);
    		}
    		else {
    			return walker.idx(size);
    		}
    	}
    }



    void reindex(Int blocks)
    {
    	Metadata* meta = this->metadata();

    	TreeLayout layout;
    	Int levels = this->compute_tree_layout(meta, layout);

    	meta->index_size() = layout.index_size;

    	for (Int block = 0; block < blocks; block++)
    	{
    		if (levels > 0)
    		{
    			this->reindex_block(meta, block, layout);
    		}
    	}
    }

    const Int& size() const {
    	return metadata()->size();
    }

    Int& size() {
    	return metadata()->size();
    }

    Int index_size() const {
    	return metadata()->index_size();
    }

    Int max_size() const {
    	return metadata()->max_size();
    }

    void dump_index(Int blocks, std::ostream& out = cout) const
    {
    	auto meta = this->metadata();

    	out<<"size_         = "<<meta->size()<<std::endl;
    	out<<"index_size_   = "<<meta->index_size()<<std::endl;

    	out<<std::endl;

    	TreeLayout layout;

    	Int levels = this->compute_tree_layout(meta->max_size(), layout);

    	if (levels > 0)
    	{
    		out<<"TreeLayout: "<<endl;

    		out<<"Level sizes: ";
    		for (Int c = 0; c <= layout.levels_max; c++) {
    			out<<layout.level_sizes[c]<<" ";
    		}
    		out<<endl;

    		out<<"Level starts: ";
    		for (Int c = 0; c <= layout.levels_max; c++) {
    			out<<layout.level_starts[c]<<" ";
    		}
    		out<<endl;
    	}

    	for (Int block = 0; block < blocks; block++)
    	{
    		out<<"++++++++++++++++++ Block: "<<block<<" ++++++++++++++++++"<<endl;

    		if (levels > 0)
    		{
    			auto indexes = this->index(block);

    			out<<"Index:"<<endl;
    			for (Int c = 0; c < meta->index_size(); c++)
    			{
    				out<<c<<": "<<indexes[c]<<endl;
    			}
    		}
    	}

    }


    void dump(Int blocks, std::ostream& out = cout, bool dump_index = true) const
    {
    	auto meta = this->metadata();

    	out<<"size_         = "<<meta->size()<<std::endl;
    	out<<"index_size_   = "<<meta->index_size()<<std::endl;

    	out<<std::endl;

		TreeLayout layout;

		Int levels = this->compute_tree_layout(meta->max_size(), layout);

    	if (dump_index)
    	{
    		if (levels > 0)
    		{
    			out<<"TreeLayout: "<<endl;

    			out<<"Level sizes: ";
    			for (Int c = 0; c <= layout.levels_max; c++) {
    				out<<layout.level_sizes[c]<<" ";
    			}
    			out<<endl;

    			out<<"Level starts: ";
    			for (Int c = 0; c <= layout.levels_max; c++) {
    				out<<layout.level_starts[c]<<" ";
    			}
    			out<<endl;
    		}
    	}

    	for (Int block = 0; block < blocks; block++)
    	{
    		out<<"++++++++++++++++++ Block: "<<block<<" ++++++++++++++++++"<<endl;

    		if (dump_index && levels > 0)
    		{
    			auto indexes = this->index(block);

				out<<"Index:"<<endl;
    			for (Int c = 0; c < meta->index_size(); c++)
    			{
    				out<<c<<": "<<indexes[c]<<endl;
    			}
    		}

    		out<<endl;

    		out<<"Values: "<<endl;

    		auto values = this->values(block);

    		for (Int c = 0; c < meta->size(); c++)
    		{
    			out<<c<<": "<<values[c]<<endl;
    		}
    	}
    }




    auto findGTForward(Int block, IndexValue val) const
    {
    	return this->find_gt(block, val);
    }

    auto findGEForward(Int block, IndexValue val) const
    {
    	return this->find_ge(block, val);
    }


    class FindResult {
    	Int idx_;
    public:
    	template <typename Fn>
    	FindResult(Fn&& fn): idx_(fn.idx()) {}

    	Int idx() const {return idx_;}
    };


    auto findForward(SearchType search_type, Int block, IndexValue val) const
    {
        if (search_type == SearchType::GT)
        {
            return FindResult(findGTForward(block, val));
        }
        else {
            return FindResult(findGEForward(block, val));
        }
    }

    auto findBackward(SearchType search_type, Int block, IndexValue val) const
    {
        if (search_type == SearchType::GT)
        {
            return FindResult(findGTBackward(block, val));
        }
        else {
            return FindResult(findGEBackward(block, val));
        }
    }



    template <typename ConsumerFn>
    Int scan(Int block, Int start, Int end, ConsumerFn&& fn) const
    {
    	auto values = this->values(block);

    	Int c;
    	for (c = start; c < end; c++)
    	{
    		fn(c, values[c]);
    	}

    	return c;
    }

    template <typename T>
    void read(Int block, Int start, Int end, T* values) const
    {
    	MEMORIA_ASSERT(start, >=, 0);
    	MEMORIA_ASSERT(start, <=, end);
    	MEMORIA_ASSERT(end, <=, size());

    	scan(block, start, end, [&](Int c, auto value){
    		values[c - start] = value;
    	});
    }


protected:
    void reindex_block(const Metadata* meta, Int block, TreeLayout& layout)
    {
    	auto values = this->values(block);
    	auto indexes = this->index(block);
    	layout.indexes = indexes;

    	Int levels = layout.levels_max + 1;

    	Int level_start = layout.level_starts[levels - 1];
    	Int level_size = layout.level_sizes[levels - 1];

    	Int size = meta->size();

    	for (int i = 0; i < level_size; i++)
    	{
    		Int window_end = (i + 1) << ValuesPerBranchLog2;

    		Int end = window_end <= size ? window_end : size;

    		indexes[level_start + i] = values[end - 1];
    	}

    	for (Int level = levels - 1; level > 0; level--)
    	{
    		Int previous_level_start = layout.level_starts[level - 1];
    		Int previous_level_size  = layout.level_sizes[level - 1];

    		Int current_level_start  = layout.level_starts[level];

    		Int current_level_size = layout.level_sizes[level];

    		for (int i = 0; i < previous_level_size; i++)
    		{
    			Int window_end 	= ((i + 1) << BranchingFactorLog2);

    			Int end = (window_end <= current_level_size ? window_end : current_level_size) + current_level_start;

    			indexes[previous_level_start + i] = indexes[end - 1];
    		}
    	}
    }
};


}


#endif