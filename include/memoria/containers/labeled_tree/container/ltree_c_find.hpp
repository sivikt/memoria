
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef MEMORIA_CONTAINERS_LBLTREE_C_FIND_HPP
#define MEMORIA_CONTAINERS_LBLTREE_C_FIND_HPP

#include <memoria/core/container/container.hpp>
#include <memoria/containers/labeled_tree/ltree_names.hpp>
#include <memoria/containers/labeled_tree/ltree_tools.hpp>
#include <memoria/containers/seq_dense/seqd_walkers.hpp>

#include <type_traits>

namespace memoria    {




MEMORIA_CONTAINER_PART_BEGIN(memoria::louds::CtrFindName)

    typedef TypesType                                                           Types;
    typedef typename Base::Iterator                                             Iterator;
    typedef typename Base::Types::LabelsTuple                                   LabelsTuple;
    typedef typename Base::Types::NodeBaseG                                     NodeBaseG;
    typedef typename Base::LeafDispatcher                                       LeafDispatcher;

    Iterator select0(BigInt rank)
    {
        return self().select(0, rank);
    }

    Iterator select1(BigInt rank)
    {
        return self().select(1, rank);
    }

    BigInt rank1(BigInt idx)
    {
        return self().rank(idx + 1, 1);
    }

    BigInt rank0(BigInt idx)
    {
        return self().rank(idx + 1, 0);
    }


    struct LabelsFn {

        LabelsTuple labels_;


        template <Int Idx, typename SeqTypes>
        void stream(const PkdFSSeq<SeqTypes>* seq, Int idx)
        {}

        template <Int Idx, typename StreamTypes>
        void stream(const PackedFSEArray<StreamTypes>* labels, Int idx)
        {
            if (labels)
            {
                std::get<Idx - 1>(labels_) = labels->value(idx);
            }
            else {
                std::get<Idx - 1>(labels_) = 0;
            }
        }


        template <Int Idx, typename StreamTypes>
        void stream(const PkdVTree<StreamTypes>* labels, Int idx)
        {
            if (labels)
            {
                std::get<Idx - 1>(labels_) = labels->value(0, idx);
            }
            else {
                std::get<Idx - 1>(labels_) = 0;
            }
        }


        template <typename NTypes>
        void treeNode(const LeafNode<NTypes>* node, Int label_idx)
        {
            node->processAll(*this, label_idx);
        }
    };

    LabelsTuple getLabels(const NodeBaseG& leaf, Int idx) const
    {
        LabelsFn fn;

        LeafDispatcher::dispatchConst(leaf, fn, idx);

        return fn.labels_;
    }

    LabelsTuple labels(const LoudsNode& node)
    {
        return self().seek(node.node()).labels();
    }

MEMORIA_CONTAINER_PART_END

}


#endif
