
// Copyright Victor Smirnov 2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_CONTAINERS_MAPX_CTR_INSERT_HPP
#define _MEMORIA_CONTAINERS_MAPX_CTR_INSERT_HPP


#include <memoria/containers/mapx/mapx_names.hpp>
#include <memoria/core/container/container.hpp>
#include <memoria/core/container/macros.hpp>

#include <memoria/containers/mapx/mapx_tools.hpp>


#include <vector>

namespace memoria    {

MEMORIA_CONTAINER_PART_BEGIN(memoria::mapx::CtrInsertName)

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

    typedef typename Types::Entry                                               MapEntry;

    template <typename Entry>
    struct InsertIntoLeafFn {

        const Entry& entry_;
        Accumulator& sums_;

        InsertIntoLeafFn(const Entry& entry, Accumulator& sums):
            entry_(entry), sums_(sums)
        {}

        template <Int StreamIdx, typename Stream>
        void stream(Stream* stream, Int idx)
        {
            MEMORIA_ASSERT_TRUE(stream);


        }


        template <typename NTypes>
        void treeNode(LeafNode<NTypes>* node, Int idx)
        {
            node->layout(255);
            node->template processStream<IntList<0>>(*this, idx);
        }
    };


    template <typename Entry>
    bool insertIntoLeaf(
            NodeBaseG&
            leaf,
            Int idx,
            const Entry& entry,
            Accumulator& sums,
            bool adjust_next
    );

    template <typename Entry>
    void insertEntry(Iterator& iter, const Entry& entry, bool adjust_next = true);


    template <typename DataType>
    struct AddLeafSingleFn: bt1::NodeWalkerBase<AddLeafSingleFn<DataType>, IntList<0>, IntList<0>>
    {
        const bt::SingleIndexUpdateData<DataType>& element_;

        AddLeafSingleFn(const bt::SingleIndexUpdateData<DataType>& element): element_(element) {}

        template <Int Idx, typename Stream>
        void stream(Stream* stream, Int idx)
        {
            MEMORIA_ASSERT_TRUE(stream);
            stream->addValue(element_.index() - 1, idx, element_.delta());
        }
    };


    template <typename DataType>
    void updateLeafNode(
            NodeBaseG& node,
            Int idx,
            const bt::SingleIndexUpdateData<DataType>& sums,
            std::function<void (Int, Int)> fn
        );

    template <typename DataType>
    void updateUp(
            NodeBaseG& node,
            Int idx,
            const bt::SingleIndexUpdateData<DataType>& sums,
            std::function<void (Int, Int)> fn
        );


    void initLeaf(NodeBaseG& node) const
    {
        auto& self = this->self();

        self.updatePageG(node);
        self.layoutNode(node, 1);
    }

    Iterator insertIFNotExists(Key key)
    {
        Iterator iter = self().findGE(0, key, 1);

        if (iter.isEnd() || key != iter.key())
        {
            MapEntry entry;
            entry.key() = key;

            self().insertEntry(iter, entry);

            iter--;
        }
        else {
            throw Exception(MA_SRC, "Inserted Key already exists");
        }

        return iter;
    }





MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::mapx::CtrInsertName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS


M_PARAMS
template <typename Entry>
bool M_TYPE::insertIntoLeaf(
        NodeBaseG& leaf,
        Int idx,
        const Entry& entry,
        Accumulator& sums,
        bool adjust_next
    )
{
    auto& self = this->self();

    self.updatePageG(leaf);

    InsertIntoLeafFn<Entry> fn(entry, sums, adjust_next);

    LeafDispatcher::dispatch(leaf, fn, idx);

    return fn.next_entry_updated_;
}



M_PARAMS
template <typename Entry>
void M_TYPE::insertEntry(Iterator& iter, const Entry& entry, bool adjust_next)
{
    auto& self      = this->self();
    NodeBaseG& leaf = iter.leaf();
    Int& idx        = iter.idx();

    if (!self.checkCapacities(leaf, {1}))
    {
        iter.split();
    }

    Entry tmp_entry = entry;

    tmp_entry.key() -= std::get<0>(iter.prefixes())[1];

    Accumulator sums;

    bool result = self.insertIntoLeaf(leaf, idx, tmp_entry, sums, adjust_next);

    self.addTotalKeyCount(Position::create(0, 1));

    if (adjust_next)
    {
        if (result)
        {
            std::get<0>(sums)[1] = 0;

            self.updateParent(leaf, sums);

            iter++;
        }
        else {
            self.updateParent(leaf, sums);

            if (iter++)
            {
                iter.adjustIndex(1, -tmp_entry.key());
            }
        }
    }
    else {
        self.updateParent(leaf, sums);
        iter++;
    }
}


M_PARAMS
template <typename DataType>
void M_TYPE::updateLeafNode(
                NodeBaseG& node,
                Int idx,
                const bt::SingleIndexUpdateData<DataType>& sums,
                std::function<void (Int, Int)> fn
    )
{
    auto& self = this->self();

    self.updatePageG(node);

    LeafDispatcher::dispatch(node, AddLeafSingleFn<DataType>(sums), idx);
}


M_PARAMS
template <typename DataType>
void M_TYPE::updateUp(
        NodeBaseG& node,
        Int idx,
        const bt::SingleIndexUpdateData<DataType>& counters,
        std::function<void (Int, Int)> fn
    )
{
    auto& self = this->self();

    self.updateLeafNode(node, idx, counters, fn);
    self.updateParent(node, counters);
}


#undef M_PARAMS
#undef M_TYPE

}


#endif
