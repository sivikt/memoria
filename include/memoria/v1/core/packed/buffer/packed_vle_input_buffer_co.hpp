
// Copyright 2015 Victor Smirnov
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

#include <memoria/v1/core/packed/tree/vle/packed_vle_quick_tree_base.hpp>
#include <memoria/v1/core/packed/tree/vle/packed_vle_tools.hpp>

#include <memoria/v1/core/packed/array/packed_vle_array_base.hpp>

#include <memoria/v1/core/tools/assert.hpp>

namespace memoria {
namespace v1 {


template <Int Blocks>
class PkdVLEInputBufferMetadata {
    using SizesT = core::StaticVector<Int, Blocks>;

    Int size_;

    SizesT data_size_;
    SizesT max_data_size_;

public:
    PkdVLEInputBufferMetadata() = default;

    Int& size() {return size_;}
    const Int& size() const {return size_;}

    SizesT& data_size() {return data_size_;}
    const SizesT& data_size() const {return data_size_;}

    SizesT& max_data_size() {return max_data_size_;}
    const SizesT& max_data_size() const {return max_data_size_;}

    Int& data_size(Int block) {return data_size_[block];}
    const Int& data_size(Int block) const {return data_size_[block];}

    Int& max_data_size(Int block) {return max_data_size_[block];}
    const Int& max_data_size(Int block) const {return max_data_size_[block];}

    SizesT capacity() const {
        return max_data_size_ - data_size_;
    }

    Int capacity(Int block) const {
        return max_data_size_[block] - data_size_[block];
    }

    template <Int, typename, template <typename> class Codec, Int, Int, typename> friend class PkdVLEArrayBase;
};


template <typename Types>
class PkdVLEColumnOrderInputBuffer: public PkdVLEArrayBase<
    Types::Blocks,
    typename Types::Value,
    Types::template Codec,
    Types::BranchingFactor,
    Types::ValuesPerBranch,
    PkdVLEInputBufferMetadata<Types::Blocks>
> {

    using Base = PkdVLEArrayBase<
            Types::Blocks,
            typename Types::Value,
            Types::template Codec,
            Types::BranchingFactor,
            Types::ValuesPerBranch,
            PkdVLEInputBufferMetadata<Types::Blocks>
    >;

    using MyType = PkdVLEColumnOrderInputBuffer<Types>;

public:
    using Base::BlocksStart;
    using Base::SegmentsPerBlock;
    using Base::compute_tree_layout;
    using Base::reindex_block;
    using Base::offsets_segment_size;
    using Base::locate;

    using Base::METADATA;
    using Base::DATA_SIZES;

    using Base::VALUES;
    using Base::OFFSETS;
    using Base::SIZE_INDEX;
    using Base::BITS_PER_DATA_VALUE;
    using Base::ValuesPerBranch;


    using typename Base::Metadata;
    using typename Base::TreeLayout;
    using typename Base::OffsetsType;
    using typename Base::ValueData;



    using typename Base::Codec;

    static constexpr UInt VERSION = 1;
    static constexpr Int Blocks = Types::Blocks;

    static constexpr Int SafetyMargin = (1024 * 8) / Codec::ElementSize;

    using FieldsList = MergeLists<
                typename Base::FieldsList,
                ConstValue<UInt, VERSION>,
                ConstValue<UInt, Blocks>
    >;

    using IndexValue = typename Types::IndexValue;
    using Value      = typename Types::Value;

    using Values = core::StaticVector<IndexValue, Blocks>;

    using InputBuffer   = MyType;
    using InputType     = Values;

    using SizesT = core::StaticVector<Int, Blocks>;

    class AppendState {
        SizesT pos_;
        Int size_ = 0;

        Metadata* meta_;
        using VValueData = core::StaticVector<ValueData*, Blocks>;

        VValueData values_;

    public:
        AppendState() {}
        AppendState(Metadata* meta): meta_(meta) {}


        SizesT& pos() {return pos_;}
        const SizesT& pos() const {return pos_;}

        Int& size() {return size_;}
        const Int& size() const {return size_;}

        VValueData& values() {return values_;}
        const VValueData& values() const {return values_;}

        Metadata* meta() {return meta_;}
    };

    void init(const SizesT& sizes)
    {
        Base::init(block_size(sizes), Blocks * SegmentsPerBlock + BlocksStart);

        Metadata* meta = this->template allocate<Metadata>(METADATA);

        meta->size() = 0;

        for (Int block = 0; block < Blocks; block++)
        {
            Int capacity        = sizes[block] + SafetyMargin;
            Int offsets_size    = offsets_segment_size(capacity);
            Int index_size      = this->index_size(capacity);
            Int values_segment_length = this->value_segment_size(capacity);

            meta->max_data_size(block) = capacity;

            this->resizeBlock(block * SegmentsPerBlock + SIZE_INDEX + BlocksStart, index_size * sizeof(Int));
            this->resizeBlock(block * SegmentsPerBlock + OFFSETS + BlocksStart, offsets_size);
            this->resizeBlock(block * SegmentsPerBlock + VALUES + BlocksStart, values_segment_length);
        }
    }


    static Int block_size(Int capacity)
    {
        return Base::block_size_equi(Blocks, capacity + SafetyMargin);
    }


    static Int block_size(const SizesT& capacity)
    {
        Int metadata_length = Base::roundUpBytesToAlignmentBlocks(sizeof(Metadata));
        Int data_sizes_length = Base::roundUpBytesToAlignmentBlocks(Blocks * sizeof(Int));


        Int segments_length = 0;

        for (Int block = 0; block < Blocks; block++)
        {
            Int block_capacity  = capacity[block] + SafetyMargin;

            Int index_size      = MyType::index_size(block_capacity);
            Int sizes_length    = Base::roundUpBytesToAlignmentBlocks(index_size * sizeof(Int));

            Int values_length   = Base::roundUpBitsToAlignmentBlocks(block_capacity * BITS_PER_DATA_VALUE);

            Int offsets_length  = offsets_segment_size(block_capacity);

            segments_length += values_length + offsets_length + sizes_length;
        }

        return PackedAllocator::block_size (
                metadata_length +
                data_sizes_length +
                segments_length,
                Blocks * SegmentsPerBlock + BlocksStart
        );
    }


    bool has_capacity_for(const SizesT& sizes) const
    {
        auto meta = this->metadata();

        for (Int block = 0; block < Blocks; block++)
        {
            if(sizes[block] > meta->max_data_size(block) - SafetyMargin)
            {
                return false;
            }
        }

        return true;
    }

    template <typename Buffer>
    bool has_capacity_for(const Buffer& buffer, Int start, Int length) const
    {
        auto meta = this->metadata();

        Codec codec;

        SizesT sizes;

        for (Int c = start; c < start + length; c++)
        {
            Values entry(buffer[c]);

            for (Int block = 0; block < Blocks; block++)
            {
                sizes[block] += codec.length(entry[block]);
            }
        }

        for (Int block = 0; block < Blocks; block++)
        {
            if(sizes[block] > meta->max_data_size(block) - SafetyMargin)
            {
                return false;
            }
        }

        return true;
    }


    Int block_size() const
    {
        return Base::block_size();
    }

    SizesT data_capacity() const
    {
        return this->metadata()->max_data_size();
    }

    void copyTo(MyType* other) const
    {
        auto meta = this->metadata();
        auto other_meta = other->metadata();

        other_meta->size()      = meta->size();
        other_meta->data_size() = meta->data_size();

        Codec codec;

        for (int b = 0; b < Blocks; b++)
        {
            codec.copy(this->values(b), 0, other->values(b), 0, meta->data_size(b));
        }
    }



    Value value(Int block, Int idx) const
    {
        auto meta = this->metadata();

        MEMORIA_V1_ASSERT(idx, >=, 0);
        MEMORIA_V1_ASSERT(idx, <, this->size());

        Int data_size     = meta->data_size(block);
        auto values       = this->values(block);
        TreeLayout layout = this->compute_tree_layout(meta->max_data_size(block));

        Int start_pos     = this->locate(layout, values, block, idx, data_size).idx;

        MEMORIA_V1_ASSERT(start_pos, <, data_size);

        Codec codec;
        Value value;

        codec.decode(values, value, start_pos);

        return value;
    }


    static Int empty_size()
    {
        return block_size(0);
    }

    void reindex()
    {
        auto metadata = this->metadata();
        for (Int block = 0; block < Blocks; block++)
        {
            auto data_size = metadata->data_size(block);
            TreeLayout layout = this->compute_tree_layout(metadata->max_data_size(block));
            Base::reindex_block(block, layout, data_size);
        }
    }



    bool check_capacity(Int size) const
    {
        MEMORIA_V1_ASSERT_TRUE(size >= 0);

        auto alloc = this->allocator();

        Int total_size          = this->size() + size;
        Int total_block_size    = MyType::block_size(total_size);
        Int my_block_size       = alloc->element_size(this);
        Int delta               = total_block_size - my_block_size;

        return alloc->free_space() >= delta;
    }

    void reset() {
        auto meta = this->metadata();
        meta->data_size().clear();
        meta->size() = 0;
    }

    Values get_values(Int idx) const
    {
        Values v;

        for (Int i = 0; i < Blocks; i++)
        {
            v[i] = this->value(i, idx);
        }

        return v;
    }

    Value get_values(Int idx, Int index) const
    {
        return this->value(index, idx);
    }

    Value getValue(Int index, Int idx) const
    {
        return this->value(index, idx);
    }

    SizesT positions(Int idx) const
    {
        auto metadata = this->metadata();

        MEMORIA_V1_ASSERT(idx, >=, 0);
        MEMORIA_V1_ASSERT(idx, <=, this->size());

        SizesT pos;
        for (Int block = 0; block < Blocks; block++)
        {
            Int data_size       = metadata->data_size(block);
            auto values         = this->values(block);
            TreeLayout layout   = compute_tree_layout(metadata->max_data_size(block));

            pos[block] = this->locate(layout, values, block, idx, data_size).idx;
        }

        return pos;
    }


    template <typename Adaptor>
    static SizesT calculate_size(Int size, Adaptor&& fn)
    {
        Codec codec;
        SizesT sizes;

        for (Int c = 0; c < size; c++) {
            for (Int b = 0; b < Blocks; b++)
            {
                sizes[b] += codec.length(fn(b, c));
            }
        }

        return sizes;
    }

    void fill(const ValueData* data, Int size, const SizesT& data_size)
    {
        auto meta = this->metadata();

        meta->size() = size;

        size_t acc = 0;

        for (Int blk = 0; blk < Blocks; blk++)
        {
            meta->data_size(blk) = data_size[blk];
            auto size            = data_size[blk];

            CopyBuffer(data + acc, this->values(blk), size);
            acc += size;
        }
    }

    AppendState append_state()
    {
        auto meta = this->metadata();

        AppendState state(meta);

        state.pos()  = meta->data_size();
        state.size() = meta->size();

        for (Int b = 0; b < Blocks; b++) {
            state.values()[b] = this->values(b);
        }

        return state;
    }


    template <typename IOBuffer>
    bool append_entry_from_iobuffer(AppendState& state, IOBuffer& buffer)
    {
        Codec codec;

        auto meta = state.meta();

        for (Int block = 0; block < Blocks; block++)
        {
            size_t capacity = meta->capacity(block);

            auto ptr = buffer.array();
            auto pos = buffer.pos();

            size_t len = codec.length(ptr, pos, -1);

            if (len <= capacity)
            {
                Int& data_pos = meta->data_size(block);
                codec.copy(ptr, pos, state.values()[block], data_pos, len);
                data_pos += len;
                buffer.skip(len);
            }
            else {
                return false;
            }
        }

        state.size()++;
        state.pos() = meta->data_size();
        this->size()++;

        return true;
    }



    void restore(const AppendState& state)
    {
        auto meta = this->metadata();

        meta->size()        = state.size();
        meta->data_size()   = state.pos();
    }





    template <typename Adaptor>
    Int append(Int size, Adaptor&& adaptor)
    {
        Codec codec;

        auto metadata = this->metadata();

        using DataSizesT = core::StaticVector<size_t, Blocks>;

        DataSizesT positions, limits;
        ValueData* values[Blocks];

        for (Int block = 0; block < Blocks; block++)
        {
            values[block]    = this->values(block);
            positions[block] = metadata->data_size(block);
            limits[block]    = metadata->max_data_size(block) - SafetyMargin;
        }

        Int c;
        for (c = 0; c < size && positions.ltAll(limits); c++)
        {
            for (Int block = 0; block < Blocks; block++)
            {
                const auto& value = adaptor(block, c);
                positions[block] += codec.encode(values[block], value, positions[block]);
            }
        }

        for (Int block = 0; block < Blocks; block++)
        {
            metadata->data_size(block) = positions[block];
        }

        metadata->size() += c;

        return c;
    }




    void check_indexless(Int block, Int data_size) const
    {
        Int offsets_size = this->element_size(block * SegmentsPerBlock + Base::OFFSETS + BlocksStart);
        MEMORIA_V1_ASSERT(this->element_size(block * SegmentsPerBlock + SIZE_INDEX + BlocksStart), ==, 0);

        if (data_size > 0)
        {
            MEMORIA_V1_ASSERT(offsets_size, ==, sizeof(OffsetsType));
            MEMORIA_V1_ASSERT(this->offset(block, 0), ==, 0);
        }
        else {
            MEMORIA_V1_ASSERT(offsets_size, ==, 0);
        }

        MEMORIA_V1_ASSERT(data_size, <=, (Int)ValuesPerBranch);
    }


    void check() const
    {
        auto metadata = this->metadata();

        for (Int block = 0; block < Blocks; block++)
        {
            Int data_size     = metadata->data_size(block);
            Int max_data_size = metadata->max_data_size(block);
            TreeLayout layout = this->compute_tree_layout(max_data_size);

            if (layout.levels_max >= 0)
            {
                Base::check_block(block, layout, data_size);
            }
            else {
                check_indexless(block, max_data_size);
            }
        }
    }


    template <typename ConsumerFn>
    Int scan(Int block, Int start, Int end, ConsumerFn&& fn) const
    {
        auto meta = this->metadata();

        auto values = this->values(block);
        size_t data_size = meta->data_size(block);

        TreeLayout layout = this->compute_tree_layout(meta->max_data_size(block));
        size_t pos = this->locate(layout, values, block, start, data_size).idx;


        Codec codec;

        Int c;
        for (c = start; c < end && pos < data_size; c++)
        {
            Value value;
            auto len = codec.decode(values, value, pos);
            fn(c, value);
            pos += len;
        }

        return c;
    }


    template <typename ConsumerFn>
    SizesT scan(Int start, Int end, ConsumerFn&& fn) const
    {
        auto metadata = this->metadata();

        MEMORIA_V1_ASSERT(start, >=, 0);
        MEMORIA_V1_ASSERT(end, >=, start);
        MEMORIA_V1_ASSERT(end, <=, metadata->size());

        Codec codec;

        SizesT position;
        const ValueData* values[Blocks];

        for (Int block = 0; block < Blocks; block++)
        {
            values[block] = this->values(block);
            auto data_size = metadata->data_size(block);

            TreeLayout layout = this->compute_tree_layout(metadata->max_data_size(block));
            position[block] = this->locate(layout, values[block], block, start, data_size).idx;
        }

        for (Int c = start; c < end; c++)
        {
            Values values_data;

            for (Int block = 0; block < Blocks; block++)
            {
                position[block] += codec.decode(values[block], values_data[block], position[block]);
            }

            fn(values_data);
        }

        return position;
    }

    void dump(std::ostream& out = cout) const
    {
        auto meta = this->metadata();
        auto size = meta->size();

        out<<"size_         = "<<size<<std::endl;
        out<<"block_size_   = "<<this->block_size()<<std::endl;

        for (Int block = 0; block < Blocks; block++) {
            out<<"data_size_["<<block<<"] = "<<this->data_size(block)<<std::endl;
        }

        for (Int block = 0; block < Blocks; block++)
        {
            out<<"++++++++++++++++++ Block: "<<block<<" ++++++++++++++++++"<<endl;

            auto data_size  = this->data_size(block);
            auto index_size = this->index_size(data_size);

            out<<"index_size_   = "<<index_size<<std::endl;

            TreeLayout layout = this->compute_tree_layout(data_size);

            if (layout.levels_max >= 0)
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

                auto size_indexes = this->size_index(block);

                out<<"Index:"<<endl;
                for (Int c = 0; c < index_size; c++)
                {
                    out<<c<<": "<<size_indexes[c]<<std::endl;
                }
            }

            out<<endl;

            out<<"Offsets: ";
            for (Int c = 0; c <= this->divUpV(data_size); c++) {
                out<<this->offset(block, c)<<" ";
            }
            out<<endl;
        }

        out<<"Values: "<<endl;

        const ValueData* values[Blocks];
        size_t block_pos[Blocks];

        for (Int block = 0; block < Blocks; block++) {
            values[block] = this->values(block);
            block_pos[block] = 0;
        }

        Codec codec;
        for (Int c = 0; c < size; c++)
        {
            out<<c<<": "<<c<<" ";
            for (Int block = 0; block < Blocks; block++)
            {
                Value value;
                auto len = codec.decode(values[block], value, block_pos[block]);

                out<<"  ("<<block_pos[block]<<") "<<value;
                block_pos[block] += len;
            }
            out<<endl;
        }
    }

    void generateDataEvents(IPageDataEventHandler* handler) const
    {
        Base::generateDataEvents(handler);

        handler->startStruct();
        handler->startGroup("VLE_COLUN_ORDER_INPUT_BUFFER");

        auto meta = this->metadata();

        handler->value("SIZE",      &meta->size());
        handler->value("DATA_SIZE", &meta->data_size(0), Blocks);

        handler->startGroup("INDEXES", Blocks);

        for (Int block = 0; block < Blocks; block++)
        {
            Int index_size = this->index_size(this->data_size(block));

            handler->startGroup("BLOCK_INDEX", block);

            auto size_indexes  = this->size_index(block);

            for (Int c = 0; c < index_size; c++)
            {
                BigInt indexes[] = {
                        size_indexes[c]
                };

                handler->value("INDEX", indexes, 1);
            }

            handler->endGroup();
        }

        handler->endGroup();


        handler->startGroup("DATA", meta->size());

        const ValueData* values[Blocks];
        for (Int b = 0; b < Blocks; b++) {
            values[b] = this->values(b);
        }

        size_t positions[Blocks];
        for (auto& p: positions) p = 0;

        Int size = this->size();

        Codec codec;

        for (Int idx = 0; idx < size; idx++)
        {
            Value values_data[Blocks];
            for (Int block = 0; block < Blocks; block++)
            {
                auto len = codec.decode(values[block], values_data[block], positions[block]);
                positions[block] += len;
            }

            handler->value("ARRAY_ITEM", PageValueProviderFactory::provider(Blocks, [&](Int idx) {
                return values_data[idx];
            }));
        }

        handler->endGroup();

        handler->endGroup();

        handler->endStruct();
    }
};




}}
