
// Copyright 2016 Victor Smirnov
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

#include <memoria/core/packed/tree/fse_max/packed_fse_max_tree.hpp>
#include <memoria/core/packed/sseq/packed_fse_searchable_seq.hpp>

#include <memoria/core/packed/tree/fse_max/packed_fse_optmax_tree_so.hpp>

#include <memoria/core/tools/optional.hpp>

namespace memoria {

template <typename Types> class PkdFMOTree;

template <typename ValueT, int32_t kBlocks = 1, int32_t kBranchingFactor = PackedTreeBranchingFactor, int32_t kValuesPerBranch = PackedTreeBranchingFactor>
using PkdFMOTreeT = PkdFMOTree<PkdFMTreeTypes<ValueT, kBlocks, kBranchingFactor, kValuesPerBranch>>;


template <typename Types>
class PkdFMOTree: public PackedAllocator {

    using Base      = PackedAllocator;
    using MyType    = PkdFMOTree<Types>;

public:

    static constexpr uint32_t VERSION = 1;
    static constexpr int32_t Blocks = Types::Blocks;

    using Tree      = PkdFMTree<Types>;
    using Bitmap    = PkdFSSeq<typename PkdFSSeqTF<1>::Type>;

    using FieldsList = MergeLists<
                typename Base::FieldsList,
                typename Tree::FieldsList,
                typename Bitmap::FieldsList,
                ConstValue<uint32_t, VERSION>
    >;



    enum {BITMAP, TREE, STRUCTS_NUM__};


    using DataType      = typename Tree::DataType;
    using IndexDataType = typename Tree::IndexDataType;
    using CxxValue      = typename Tree::Value;

    using IndexValue    = CxxValue;
    using TreeValue     = CxxValue;
    using TreeValues    = typename Tree::Values;

    using Value      = OptionalT<CxxValue>;
    using Values     = core::StaticVector<Value, Blocks>;
    using SizesT     = typename Tree::SizesT;

    using AccumValue = Value;

    using ExtData = typename DataTypeTraits<DataType>::ExtData;
    using SparseObject = PackedFSEOptMaxTreeSO<ExtData, MyType>;

    using Base::block_size;

    Bitmap* bitmap() {
        return this->template get<Bitmap>(BITMAP);
    }

    const Bitmap* bitmap() const {
        return this->template get<Bitmap>(BITMAP);
    }

    Tree* tree() {
        return this->template get<Tree>(TREE);
    }

    const Tree* tree() const {
        return this->template get<Tree>(TREE);
    }

    static int32_t empty_size()
    {
        int32_t parent_size = PackedAllocator::empty_size(STRUCTS_NUM__);
        return parent_size + Bitmap::empty_size() + Tree::empty_size();
    }


    static int32_t block_size(int32_t capacity)
    {
        int32_t parent_size = PackedAllocator::empty_size(STRUCTS_NUM__);
        return parent_size + Bitmap::packed_block_size(capacity) + Tree::block_size(capacity);
    }

    int32_t block_size(const MyType* other) const
    {
        return MyType::block_size(size() + other->size());
    }

    OpStatus init(int32_t capacity = 0)
    {
        if(isFail(Base::init(block_size(capacity), STRUCTS_NUM__))) {
            return OpStatus::FAIL;
        }

        int32_t bitmap_block_size = Bitmap::packed_block_size(capacity);

        Bitmap* bitmap = allocateSpace<Bitmap>(BITMAP, bitmap_block_size);
        if(isFail(bitmap)){
            return OpStatus::FAIL;
        }

        if(isFail(bitmap->init(bitmap_block_size))) {
            return OpStatus::FAIL;
        }

        Tree* tree = allocateSpace<Tree>(TREE, Tree::block_size(capacity));

        if(isFail(tree)) {
            return OpStatus::FAIL;
        }

        if(isFail(tree->init(capacity))) {
            return OpStatus::FAIL;
        }

        return OpStatus::OK;
    }

    OpStatus init(const SizesT& sizes)
    {
        return MyType::init(sizes[0]);
    }



    int32_t size() const
    {
        return bitmap()->size();
    }


    template <typename T>
    void max(core::StaticVector<T, Blocks>& accum) const
    {
        const Tree* tree = this->tree();

        int32_t size = tree->size();

        if (size > 0)
        {
            for (int32_t block = 0; block < Blocks; block++)
            {
                accum[block] = tree->value(block, size - 1);
            }
        }
        else {
            for (int32_t block = 0; block < Blocks; block++)
            {
                accum[block] = Value();
            }
        }
    }



    const Value value(int32_t block, int32_t idx) const
    {
        const Bitmap* bitmap = this->bitmap();

        if (bitmap->symbol(idx) == 1)
        {
            int32_t tree_idx = this->tree_idx(bitmap, idx);
            return tree()->value(block, tree_idx);
        }
        else {
            return Value();
        }
    }

    Values get_values(int32_t idx) const
    {
        Values v;

        auto bitmap = this->bitmap();

        if (bitmap->symbol(idx) == 1)
        {
            auto tree = this->tree();
            int32_t tree_idx = this->tree_idx(idx);

            core::OptionalAssignmentHelper(v, tree->get_values(tree_idx));
        }

        return v;
    }


    template <typename T>
    OpStatus setValues(int32_t idx, const core::StaticVector<T, Blocks>& values)
    {
        Bitmap* bitmap  = this->bitmap();
        Tree* tree      = this->tree();

        if (values[0].is_set())
        {
            TreeValues tree_values  = this->tree_values(values);
            int32_t tree_idx        = this->tree_idx(idx);

            if (bitmap->symbol(idx))
            {
                if(isFail(tree->setValues(tree_idx, tree_values))) {
                    return OpStatus::FAIL;
                }
            }
            else {
                if (isFail(tree->insert(tree_idx, tree_values))) {
                    return OpStatus::FAIL;
                }

                bitmap->symbol(idx) = 1;
                if (isFail(bitmap->reindex())) {
                    return OpStatus::FAIL;
                }
            }
        }
        else {
            int32_t tree_idx = this->tree_idx(idx);

            if (bitmap->symbol(idx))
            {
                if (isFail(tree->remove(tree_idx, tree_idx + 1))) {
                    return OpStatus::FAIL;
                }

                bitmap->symbol(idx) = 0;
                if (isFail(bitmap->reindex())) {
                    return OpStatus::FAIL;
                }
            }
            else {
                // Do nothing
            }
        }

        return OpStatus::OK;
    }


    template <typename T>
    auto findGTForward(int32_t block, const T& val) const
    {
        auto result = tree()->find_gt(block, val);

        result.set_idx(global_idx(result.local_pos()));

        return result;
    }

    template <typename T>
    auto findGTForward(int32_t block, const OptionalT<T>& val) const
    {
        auto result = tree()->find_gt(block, val.value());

        result.set_idx(global_idx(result.local_pos()));

        return result;
    }

    template <typename T>
    auto findGEForward(int32_t block, const T& val) const
    {
        auto result = tree()->find_ge(block, val.value());

        result.set_idx(global_idx(result.local_pos()));

        return result;
    }

    template <typename T>
    auto findForward(SearchType search_type, int32_t block, const T& val) const
    {
        auto result = tree()->findForward(search_type, block, val);

        result.set_idx(global_idx(result.local_pos()));

        return result;
    }

    template <typename T>
    auto findForward(SearchType search_type, int32_t block, const OptionalT<T>& val) const
    {
        auto result = tree()->findForward(search_type, block, val.value());

        result.set_idx(global_idx(result.local_pos()));

        return result;
    }


    template <typename T>
    auto findBackward(SearchType search_type, int32_t block, const T& val) const
    {
        auto result = tree()->findBackward(search_type, block, val);

        result.set_idx(global_idx(result.local_pos()));

        return result;
    }

    template <typename T>
    auto findBackward(SearchType search_type, int32_t block, const OptionalT<T>& val) const
    {
        auto result = tree()->findBackward(search_type, block, val.value());

        result.set_idx(global_idx(result.local_pos()));

        return result;
    }



    OpStatus reindex()
    {
        if(isFail(bitmap()->reindex())) {
            return OpStatus::FAIL;
        }

        return tree()->reindex();
    }

    void check() const
    {
        bitmap()->check();
        tree()->check();
    }


    OpStatus splitTo(MyType* other, int32_t idx)
    {
        Bitmap* bitmap = this->bitmap();

        int32_t tree_idx = this->tree_idx(bitmap, idx);

        if(isFail(bitmap->splitTo(other->bitmap(), idx))) {
            return OpStatus::FAIL;
        }

        if(isFail(tree()->splitTo(other->tree(), tree_idx))) {
            return OpStatus::FAIL;
        }

        return reindex();
    }

    OpStatus mergeWith(MyType* other)
    {
        if(isFail(bitmap()->mergeWith(other->bitmap()))) {
            return OpStatus::FAIL;
        }

        return tree()->mergeWith(other->tree());
    }

    OpStatus removeSpace(int32_t start, int32_t end)
    {
        return remove(start, end);
    }

    OpStatus remove(int32_t start, int32_t end)
    {
        Bitmap* bitmap = this->bitmap();

        int32_t tree_start = tree_idx(bitmap, start);
        int32_t tree_end = tree_idx(bitmap, end);

        if(isFail(bitmap->remove(start, end))) {
            return OpStatus::FAIL;
        }

        return tree()->remove(tree_start, tree_end);
    }

    template <typename T>
    OpStatus insert(int32_t idx, const core::StaticVector<T, Blocks>& values)
    {
        Bitmap* bitmap  = this->bitmap();

        if (values[0].is_set())
        {
            if(isFail(bitmap->insert(idx, 1))) {
                return OpStatus::FAIL;
            }

            TreeValues tree_values  = this->tree_values(values);
            int32_t tree_idx        = this->tree_idx(bitmap, idx);

            Tree* tree = this->tree();
            if(isFail(tree->insert(tree_idx, tree_values))) {
                return OpStatus::FAIL;
            }
        }
        else {
            if(isFail(bitmap->insert(idx, 0))) {
                return OpStatus::FAIL;
            }
        }

        return OpStatus::OK;
    }


    OpStatus insert(int32_t idx, int32_t size, std::function<const Values& (int32_t)> provider, bool reindex = true)
    {
        auto bitmap = this->bitmap();
        int32_t bitidx = 0;
        int32_t setted = 0;

        if(isFail(bitmap->insert(idx, size, [&]() {
            auto v = provider(bitidx++);
            setted += v[0].is_set();
            return v[0].is_set();
        }))) {
            return OpStatus::FAIL;
        };

        auto tree    = this->tree();
        int32_t tree_idx = this->tree_idx(bitmap, idx);

        int tidx = 0;

        TreeValues tv;

        if(isFail(tree->insert(tree_idx, setted, [&, this](int32_t) -> const auto& {
            while(true)
            {
                if (tidx < size)
                {
                    const auto& v = provider(tidx++);

                    if (v[0].is_set())
                    {
                        tv = this->tree_values(v);



                        return tv;
                    }
                }
                else {
                    MMA_THROW(Exception()) << WhatInfo(format_u8("Position {} exceeds {}", tidx, size));
                }
            }
        }))) {
            return OpStatus::FAIL;
        };

        return OpStatus::OK;
    }



    void generateDataEvents(IBlockDataEventHandler* handler) const
    {
        handler->startStruct();
        handler->startGroup("FSMO_TREE");

        bitmap()->generateDataEvents(handler);
        tree()->generateDataEvents(handler);

        handler->endGroup();
        handler->endStruct();
    }


    template <typename SerializationData>
    void serialize(SerializationData& buf) const
    {
        Base::serialize(buf);

        bitmap()->serialize(buf);
        tree()->serialize(buf);
    }


    template <typename DeserializationData>
    void deserialize(DeserializationData& buf)
    {
        Base::deserialize(buf);

        bitmap()->deserialize(buf);
        tree()->deserialize(buf);
    }

    void dump() const {
        bitmap()->dump();
        tree()->dump();
    }

protected:

    template <typename T>
    core::StaticVector<TreeValue, Blocks> tree_values(const core::StaticVector<OptionalT<T>, Blocks>& values)
    {
        core::StaticVector<TreeValue, Blocks> tv;

        for (int32_t b = 0;  b < Blocks; b++)
        {
            tv[b] = values[b].value();
        }

        return tv;
    }

    int32_t tree_idx(int32_t global_idx) const
    {
        return tree_idx(bitmap(), global_idx);
    }

    int32_t tree_idx(const Bitmap* bitmap, int32_t global_idx) const
    {
        int32_t rank = bitmap->rank(global_idx, 1);
        return rank;
    }


    int32_t global_idx(int32_t tree_idx) const
    {
        return global_idx(bitmap(), tree_idx);
    }

    int32_t global_idx(const Bitmap* bitmap, int32_t tree_idx) const
    {
        auto result = bitmap->selectFw(1, tree_idx + 1);
        return result.local_pos();
    }


};

template <typename Types>
struct PackedStructTraits<PkdFMOTree<Types>> {

    using SearchKeyDataType = typename Types::ValueDataType;
    using AccumType = typename PkdFMOTree<Types>::AccumValue;
    using SearchKeyType = typename PkdFMOTree<Types>::Value;

    static constexpr PackedDataTypeSize DataTypeSize = PackedDataTypeSize::FIXED;
    static constexpr PkdSearchType KeySearchType = PkdSearchType::MAX;
    static constexpr int32_t Blocks = PkdFMOTree<Types>::Blocks;
    static constexpr int32_t Indexes = PkdFMOTree<Types>::Blocks;
};

}