
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CONTAINERS_MAP_ITER_API_HPP
#define _MEMORIA_CONTAINERS_MAP_ITER_API_HPP

#include <memoria/core/types/types.hpp>

#include <memoria/containers/map/map_names.hpp>
#include <memoria/core/container/iterator.hpp>
#include <memoria/core/container/macros.hpp>

#include <memoria/core/packed/map/packed_fse_map.hpp>
#include <memoria/core/packed/map/packed_fse_mark_map.hpp>

#include <iostream>

namespace memoria    {


MEMORIA_ITERATOR_PART_BEGIN(memoria::map::ItrApiName)

    typedef Ctr<typename Types::CtrTypes>                                       Container;

    typedef typename Base::Allocator                                            Allocator;
    typedef typename Base::NodeBase                                             NodeBase;
    typedef typename Base::NodeBaseG                                            NodeBaseG;
    typedef typename Base::TreePath                                             TreePath;

    typedef typename Container::Value                                           Value;
    typedef typename Container::Key                                             Key;
    typedef typename Container::Element                                         Element;
    typedef typename Container::Accumulator                                     Accumulator;
    typedef typename Container::Position                                        Position;

    typedef typename Container::Types::Pages::LeafDispatcher                    LeafDispatcher;


    void updateUp(const Accumulator& keys)
    {
        auto& self = this->self();

        self.ctr().updateUp(self.leaf(), self.idx(), keys, [&](Int, Int idx) {
            self.idx() = idx;
            self.updatePrefix();
        });
    }


    struct KeyFn {
        Key value_ = 0;

        template <Int Idx, typename StreamTypes>
        void stream(const PackedFSEMap<StreamTypes>* map, Int idx)
        {
            if (map != nullptr)
            {
                value_ = map->tree()->value(0, idx);
            }
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PackedVLEMap<StreamTypes>* map, Int idx)
        {
            if (map != nullptr)
            {
                value_ = map->tree()->value(0, idx);
            }
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PackedFSESearchableMarkableMap<StreamTypes>* map, Int idx)
        {
        	if (map != nullptr)
        	{
        		value_ = map->tree()->value(0, idx);
        	}
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PackedFSEMarkableMap<StreamTypes>* map, Int idx)
        {
        	if (map != nullptr)
        	{
        		value_ = map->tree()->value(0, idx);
        	}
        }

        template <typename Node>
        void treeNode(const Node* node, Int idx)
        {
            node->template processStream<0>(*this, idx);
        }
    };


    Key raw_key() const
    {
        auto& self = this->self();

        KeyFn fn;

        LeafDispatcher::dispatchConst(self.leaf(), fn, self.idx());

        return fn.value_;
    }

    Key key() const
    {
        return self().prefix() + raw_key();
    }


    std::pair<Key, Value> operator*() const
    {
        return std::pair<Key, Value>(self().key(), self().value());
    }


    bool next() {
        return self().nextKey();
    }

    bool prev() {
        return self().prevKey();
    }

    BigInt prefix() const
    {
        return self().cache().prefix();
    }

    Accumulator prefixes() const {
        Accumulator acc;

        std::get<0>(acc)[0] = self().prefix();

        return acc;
    }

    Int entry_idx() const
    {
        return self().idx();
    }

//
//    struct SetValueFn {
//
//        template <Int Idx, typename StreamTypes>
//        void stream(PackedFSEMap<StreamTypes>* map, Int idx, const Value& value)
//        {
//            MEMORIA_ASSERT_TRUE(map != nullptr);
//            map->value(idx) = value;
//        }
//
//        template <Int Idx, typename StreamTypes>
//        void stream(PackedVLEMap<StreamTypes>* map, Int idx, const Value& value)
//        {
//            MEMORIA_ASSERT_TRUE(map != nullptr);
//            map->value(idx) = value;
//        }
//
//        template <Int Idx, typename StreamTypes>
//        void stream(PackedFSESearchableMarkableMap<StreamTypes>* map, Int idx, const Value& value)
//        {
//        	MEMORIA_ASSERT_TRUE(map != nullptr);
//        	map->value(idx) = value;
//        }
//
//        template <Int Idx, typename StreamTypes>
//        void stream(PackedFSEMarkableMap<StreamTypes>* map, Int idx, const Value& value)
//        {
//        	MEMORIA_ASSERT_TRUE(map != nullptr);
//        	map->value(idx) = value;
//        }
//
//        template <typename Node>
//        void treeNode(Node* node, Int idx, const Value& value)
//        {
//            node->template processStream<0>(*this, idx, value);
//        }
//    };
//
//
//    void setValue(const Value& value)
//    {
//        auto& self = this->self();
//        self.leaf().update();
//        LeafDispatcher::dispatch(self.leaf(), SetValueFn(), self.idx(), value);
//    }
//
//    class ValueAccessor {
//        MyType& iter_;
//    public:
//        ValueAccessor(MyType& iter): iter_(iter) {}
//
//        operator Value() const {
//            return iter_.getValue();
//        }
//
//        Value operator=(const Value& value) {
//            iter_.setValue(value);
//            return value;
//        }
//    };
//
//    class ConstValueAccessor {
//        const MyType& iter_;
//    public:
//        ConstValueAccessor(const MyType& iter): iter_(iter) {}
//
//        operator Value() const {
//            return iter_.getValue();
//        }
//    };
//
//
//    struct GetValueFn {
//        Value value_ = 0;
//
//        template <Int Idx, typename StreamTypes>
//        void stream(const PackedFSEMap<StreamTypes>* map, Int idx)
//        {
//            if (map != nullptr)
//            {
//                value_ = map->value(idx);
//            }
//        }
//
//        template <Int Idx, typename StreamTypes>
//        void stream(const PackedVLEMap<StreamTypes>* map, Int idx)
//        {
//            if (map != nullptr)
//            {
//                value_ = map->value(idx);
//            }
//        }
//
//        template <Int Idx, typename StreamTypes>
//        void stream(const PackedFSESearchableMarkableMap<StreamTypes>* map, Int idx)
//        {
//        	if (map != nullptr)
//        	{
//        		value_ = map->value(idx);
//        	}
//        }
//
//        template <typename Node>
//        void treeNode(const Node* node, Int idx)
//        {
//            node->template processStream<0>(*this, idx);
//        }
//    };
//
//
//
//
//    Value getValue() const
//    {
//        auto& self = this->self();
//
//        GetValueFn fn;
//
//        LeafDispatcher::dispatchConst(self.leaf(), fn, self.idx());
//
//        return fn.value_;
//    }
//
//    ValueAccessor value() {
//        return ValueAccessor(self());
//    }
//
//    ConstValueAccessor value() const {
//        return ConstValueAccessor(self());
//    }
//
//
//    void setData(const Value& value)
//    {
//        self().value() = value;
//    }




    void remove()
    {
        auto& self = this->self();

        Accumulator keys;
        self.ctr().removeMapEntry(self, keys);

        if (!self.isEnd())
        {
            self.updateUp(keys);
        }
    }



    void ComputePrefix(BigInt& accum)
    {
        auto& self = this->self();

        PrefixFn fn;

        if (self.idx() >= 0)
        {
            self.ctr().walkUp(self.leaf(), self.idx(), fn);
        }

        accum = fn.prefix_;
    }

    void ComputePrefix(Accumulator& accum)
    {
        ComputePrefix(std::get<0>(accum)[0]);
    }

    void dump(std::ostream& out = std::cout)
    {
        out<<"Prefix="<<self().cache().prefix()<<endl;
        Base::dump(out);
    }

    struct PrefixFn {
        BigInt prefix_ = 0;

        PrefixFn() {}

        template <Int Idx, typename StreamTypes>
        void stream(const PackedFSEMap<StreamTypes>* map, Int idx)
        {
            if (map != nullptr)
            {
                prefix_ += map->tree()->sum(0, idx);
            }
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PackedVLEMap<StreamTypes>* map, Int idx)
        {
            if (map != nullptr)
            {
                prefix_ += map->tree()->sum(0, idx);
            }
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PkdFTree<StreamTypes>* tree, Int idx)
        {
            if (tree != nullptr)
            {
                prefix_ += tree->sum(0, idx);
            }
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PkdVTree<StreamTypes>* tree, Int idx)
        {
            if (tree != nullptr)
            {
                prefix_ += tree->sum(0, idx);
            }
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PackedFSESearchableMarkableMap<StreamTypes>* map, Int idx)
        {
        	if (map != nullptr)
        	{
        		prefix_ += map->tree()->sum(0, idx);
        	}
        }

        template <Int Idx, typename StreamTypes>
        void stream(const PackedFSEMarkableMap<StreamTypes>* map, Int idx)
        {
        	if (map != nullptr)
        	{
        		prefix_ += map->tree()->sum(0, idx);
        	}
        }

        template <typename Node>
        void treeNode(const Node* node, Int idx)
        {
            node->template processStream<0>(*this, idx);
        }
    };

    void updatePrefix()
    {
        auto& self = this->self();

        PrefixFn fn;

        if (self.idx() >= 0)
        {
            self.ctr().walkUp(self.leaf(), self.idx(), fn);
        }

        self.cache().setup(fn.prefix_);
    }

    void split()
    {
        auto& self = this->self();

        NodeBaseG& leaf = self.leaf();
        Int& idx        = self.idx();

        Int size        = self.leaf_size(0);
        Int split_idx   = size/2;

        auto right = self.ctr().splitLeafP(leaf, Position::create(0, split_idx));

        if (idx > split_idx)
        {
            leaf = right;
            idx -= split_idx;

            self.updatePrefix();
        }
    }

MEMORIA_ITERATOR_PART_END

}

#endif
