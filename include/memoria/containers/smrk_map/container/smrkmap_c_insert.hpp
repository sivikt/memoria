
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_CONTAINERS_SMRKMAP_CTR_INSERT_HPP
#define _MEMORIA_CONTAINERS_SMRKMAP_CTR_INSERT_HPP


#include <memoria/containers/smrk_map/smrkmap_names.hpp>
#include <memoria/core/container/container.hpp>
#include <memoria/core/container/macros.hpp>

#include <memoria/core/packed/map/packed_fse_map.hpp>
#include <memoria/core/packed/map/packed_fse_smark_map.hpp>

#include <vector>

namespace memoria    {

MEMORIA_CONTAINER_PART_BEGIN(memoria::smrk_map::CtrInsertName)

    typedef typename Base::Types                                                Types;

    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Base::Iterator                                             Iterator;

    typedef typename Base::LeafDispatcher                                       LeafDispatcher;

    typedef typename Types::Key                                                 Key;
    typedef typename Types::Value                                               Value;


    typedef typename Types::Accumulator                                         Accumulator;
    typedef typename Types::Position                                            Position;

    static const Int Streams                                                    = Types::Streams;

    typedef typename Types::PageUpdateMgr                                       PageUpdateMgr;

    typedef ValuePair<Accumulator, Value>                                       Element;

    struct InsertIntoLeafFn {
        const Key&      key_;
        const Value&    value_;

        const Int       mark_;

        InsertIntoLeafFn(const Key& key, const Value& value, Int mark):
            key_(key),
            value_(value),
            mark_(mark)
        {}


        template <Int Idx, typename StreamTypes>
        void stream(PackedFSESearchableMarkableMap<StreamTypes>* map, Int idx)
        {
            MEMORIA_ASSERT_TRUE(map != nullptr);

            typename PackedFSESearchableMarkableMap<StreamTypes>::TreeKeys keys;

            keys[0] = key_;

            map->insert(idx, keys, value_, mark_);
        }


        template <typename NTypes>
        void treeNode(LeafNode<NTypes>* node, Int idx)
        {
            node->layout(1);
            node->template processStream<0>(*this, idx);
        }
    };




    bool insertIntoLeaf(NodeBaseG& leaf, Int idx, const Element& element, Int mark);

    bool insertMapEntry(Iterator& iter, const Element& element, Int mark);



    struct AddLeafFn {

        const Accumulator& element_;

        AddLeafFn(const Accumulator& element): element_(element) {}

        template <Int Idx, typename StreamTypes>
        void stream(PackedFSESearchableMarkableMap<StreamTypes>* map, Int idx)
        {
            MEMORIA_ASSERT_TRUE(map != nullptr);

            map->addValues(idx, std::get<Idx>(element_));
        }

        template <typename NTypes>
        void treeNode(LeafNode<NTypes>* node, Int idx)
        {
            node->template processStream<0>(*this, idx);
        }
    };


    void updateLeafNode(NodeBaseG& node, Int idx, const Accumulator& sums, std::function<void (Int, Int)> fn);
    void updateUp(NodeBaseG& node, Int idx, const Accumulator& sums, std::function<void (Int, Int)> fn);

    void initLeaf(NodeBaseG& node) const
    {
        self().updatePageG(node);
        self().layoutNode(node, 1);
    }


MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::smrk_map::CtrInsertName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS


M_PARAMS
bool M_TYPE::insertIntoLeaf(NodeBaseG& leaf, Int idx, const Element& element, Int mark)
{
    auto& self = this->self();

    PageUpdateMgr mgr(self);

    self.updatePageG(leaf);

    mgr.add(leaf);

    try {
        LeafDispatcher::dispatch(leaf, InsertIntoLeafFn(std::get<0>(element.first)[1], element.second, mark), idx);
        return true;
    }
    catch (PackedOOMException& e)
    {
        mgr.rollback();
        return false;
    }
}



M_PARAMS
bool M_TYPE::insertMapEntry(Iterator& iter, const Element& element, Int mark)
{
    auto& self      = this->self();
    NodeBaseG& leaf = iter.leaf();
    Int& idx        = iter.idx();

    if (!self.insertIntoLeaf(leaf, idx, element, mark))
    {
        iter.split();
        if (!self.insertIntoLeaf(leaf, idx, element, mark))
        {
            throw Exception(MA_SRC, "Second insertion attempt failed");
        }
    }

    Accumulator e = element.first;

    std::get<0>(e)[0]           = 1;
    std::get<0>(e)[2 + mark]    = 1;

    self.updateParent(leaf, e);

    self.addTotalKeyCount(Position::create(0, 1));

    self.markCtrUpdated();

    return iter++;
}



M_PARAMS
void M_TYPE::updateLeafNode(NodeBaseG& node, Int idx, const Accumulator& sums, std::function<void (Int, Int)> fn)
{
    auto& self = this->self();

    self.updatePageG(node);

    PageUpdateMgr mgr(self);

    try {
        LeafDispatcher::dispatch(node, AddLeafFn(sums), idx);
    }
    catch (PackedOOMException ex)
    {
        Position sizes = self.getNodeSizes(node);

        Position split_idx = sizes / 2;

        auto next = self.splitLeafP(node, split_idx);

        if (idx >= split_idx[0])
        {
            idx -= split_idx[0];
            fn(0, idx);
            node = next;
        }

        LeafDispatcher::dispatch(node, AddLeafFn(sums), idx);
    }
}


M_PARAMS
void M_TYPE::updateUp(NodeBaseG& node, Int idx, const Accumulator& counters, std::function<void (Int, Int)> fn)
{
    auto& self = this->self();

    self.updateLeafNode(node, idx, counters, fn);
    self.updateParent(node, counters);
}


#undef M_PARAMS
#undef M_TYPE

}


#endif
