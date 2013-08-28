
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef MEMORIA_CONTAINERS_WT_C_API_HPP
#define MEMORIA_CONTAINERS_WT_C_API_HPP

#include <memoria/core/container/container.hpp>

#include <memoria/containers/wt/wt_names.hpp>

#include <functional>

namespace memoria {


MEMORIA_CONTAINER_PART_BEGIN(memoria::wt::CtrApiName)

    typedef typename Base::Tree                                                 Tree;
    typedef typename Base::Seq                                                  Seq;

    typedef typename Tree::Iterator                                             TreeIterator;
    typedef typename Seq::Iterator                                              SeqIterator;

    void prepare()
    {
        auto& self = this->self();
        auto& tree = self.tree();

        TreeIterator iter = tree.seek(0);

        iter.insertNode(std::make_tuple(0, 0));
        iter++;
        iter.insertZero();
    }

    BigInt size()
    {
        auto& self = this->self();
        auto root = self.tree().seek(0);

        if (!root.isEof())
        {
            return std::get<1>(root.labels());
        }
        else {
            return 0;
        }
    }

    void insert(Int idx, UBigInt value)
    {
        auto& self = this->self();
        auto& tree = self.tree();

        auto root = tree.seek(0);

        insert(root, idx, value, 3);
    }

    void remove(Int idx)
    {
        auto& self = this->self();
        auto& tree = self.tree();

        auto root = tree.seek(0);

        removeValue(idx, root, 3);
    }



    UBigInt value(Int idx)
    {
        UBigInt value = 0;

        auto& self = this->self();
        auto& tree = self.tree();

        auto root = tree.seek(0);

        buildValue(idx, root, value, 3);

        return value;
    }


    BigInt rank(BigInt idx, UBigInt symbol)
    {
        auto& self = this->self();
        auto& tree = self.tree();

        auto root = tree.seek(0);

        return buildRank(root, idx, symbol, 3);
    }


    BigInt select(BigInt rank, UBigInt symbol)
    {
        auto& self = this->self();
        auto& tree = self.tree();

        auto root = tree.seek(0);

        return select(root, rank, symbol, 3) - 1;
    }

private:

    void insert(TreeIterator& node, Int idx, UBigInt value, Int level)
    {
        auto& self = this->self();
        auto& tree = self.tree();
        auto& seq  = self.seq();

        Int label = (value >> (level * 8)) & 0xFF;

        BigInt seq_base = node.template sumLabel<1>();

        seq.insert(seq_base + idx, label);
        node.addLabel(1, 1);

        BigInt rank = seq.rank(seq_base, idx + 1, label);

        if (level > 0)
        {
            auto child = self.findChild(node, label);

            if (rank == 1)
            {
                tree.newNodeAt(child, std::make_tuple(label, 0));
            }

            insert(child, rank - 1, value, level - 1);
        }
    }


    void buildValue(Int idx, TreeIterator& node, UBigInt& value, Int level)
    {
        auto& self = this->self();
        auto& seq  = self.seq();

        BigInt seq_base = node.template sumLabel<1>();
        UBigInt label   = seq.seek(seq_base + idx).symbol();
        BigInt  rank    = seq.rank(seq_base, idx + 1, label);

        value |= label << (level * 8);

        if (level > 0)
        {
            auto child = self.findChild(node, label);
            buildValue(rank - 1, child, value, level - 1);
        }
    }


    void removeValue(Int idx, TreeIterator& node, Int level)
    {
        auto& self = this->self();
        auto& tree = self.tree();
        auto& seq  = self.seq();

        BigInt node_pos = node.pos();

        BigInt seq_base = node.template sumLabel<1>();
        UBigInt label   = seq.seek(seq_base + idx).symbol();
        BigInt  rank    = seq.rank(seq_base, idx + 1, label);

        if (level > 0)
        {
            auto child = self.findChild(node, label);
            removeValue(rank - 1, child, level - 1);
        }

        node = tree.seek(node_pos);

        seq_base = node.template sumLabel<1>();
        seq.seek(seq_base + idx).remove();
        node.addLabel(1, -1);

        BigInt seq_length = std::get<1>(node.labels());

        if (seq_length == 0)
        {
            if (node.pos() > 0)
            {
                tree.removeLeaf(node);
            }
        }
    }


    BigInt select(TreeIterator& node, Int rank, UBigInt symbol, Int level)
    {
        auto& self = this->self();
        auto& seq  = self.seq();

        if (level >= 0)
        {
            Int label = (symbol >> (level * 8)) & 0xFFull;

            auto child = self.findChild(node, label);

            BigInt rnk = select(child, rank, symbol, level - 1);

            BigInt seq_base = node.template sumLabel<1>();

            BigInt pos = seq.select(seq_base, rnk, label).pos() + 1;

            return pos - seq_base;
        }
        else {
            return rank;
        }
    }


    BigInt buildRank(TreeIterator& node, BigInt idx, UBigInt symbol, Int level)
    {
        Int label  = (symbol >> (level * 8)) & 0xFFull;

        auto& self = this->self();
        auto& seq  = self.seq();

        BigInt seq_base = node.template sumLabel<1>();

        BigInt rank = seq.rank(seq_base, idx + 1, label);

        if (level > 0)
        {
            auto child = self.findChild(node, label);
            return buildRank(child, rank - 1, symbol, level - 1);
        }
        else {
            return rank;
        }
    }


MEMORIA_CONTAINER_PART_END

}


#endif
