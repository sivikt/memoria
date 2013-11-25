
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_CONTAINER_VECTORMAP_C_TOOLS_HPP
#define _MEMORIA_CONTAINER_VECTORMAP_C_TOOLS_HPP


#include <memoria/prototypes/bt/bt_macros.hpp>
#include <memoria/containers/vector_map/vmap_names.hpp>
#include <memoria/containers/vector_map/vmap_tools.hpp>
#include <memoria/core/container/container.hpp>
#include <memoria/core/container/macros.hpp>



namespace memoria    {

using namespace memoria::bt;

MEMORIA_CONTAINER_PART_BEGIN(memoria::vmap::CtrToolsName)

    typedef typename Base::Types                                                Types;
    typedef typename Base::Allocator                                            Allocator;

    typedef typename Base::ID                                                   ID;

    typedef typename Types::NodeBase                                            NodeBase;
    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Base::Iterator                                             Iterator;

    typedef typename Base::NodeDispatcher                                       NodeDispatcher;
    typedef typename Base::RootDispatcher                                       RootDispatcher;
    typedef typename Base::LeafDispatcher                                       LeafDispatcher;
    typedef typename Base::NonLeafDispatcher                                    NonLeafDispatcher;

    typedef typename Types::Key                                                 Key;
    typedef typename Types::Value                                               Value;

    typedef typename Base::Metadata                                             Metadata;

    typedef typename Types::Accumulator                                         Accumulator;
    typedef typename Types::Position                                            Position;


    static const Int Streams                                                    = Types::Streams;

    typedef typename Base::BTNodeTraits                                 BTNodeTraits;


    template <typename Node>
    Int getNodeTraitsFn(BTNodeTraits trait, Int page_size) const
    {
        switch (trait)
        {
        case BTNodeTraits::MAX_CHILDREN:
            return Node::max_tree_size_for_block(page_size, true); break;

        default: throw DispatchException(MEMORIA_SOURCE, "Unknown static node trait value", (Int)trait);
        }
    }

    MEMORIA_CONST_STATIC_FN_WRAPPER_RTN(GetNodeTraitsFn, getNodeTraitsFn, Int);

    Int getNodeTraitInt(BTNodeTraits trait, bool root, bool leaf) const
    {
        Int page_size = self().ctr().getRootMetadata().page_size();
        return NodeDispatcher::template dispatchStaticRtn<BranchNode>(root, leaf, GetNodeTraitsFn(me()), trait, page_size);
    }


    void sumLeafKeys(const NodeBase *node, Int from, Int count, Accumulator& keys) const
    {
        VectorAdd(keys, LeafDispatcher::dispatchConstRtn(node, typename Base::WrappedCtr::SumKeysFn(&self().ctr()), from, count));
    }

//    template <typename Node>
//    void setAndReindexFn(Node* node, Int idx, const Element& element) const
//    {
//        node->value(idx) = element.second;
//
//        if (idx == node->children_count() - 1)
//        {
//            node->reindexAll(idx, idx + 1);
//        }
//        else {
//            node->reindexAll(idx, node->children_count());
//        }
//    }
//
//    MEMORIA_CONST_FN_WRAPPER(SetAndReindexFn, setAndReindexFn);
//
//
//    void setLeafDataAndReindex(NodeBaseG& node, Int idx, const Element& element) const
//    {
//        self().ctr().setKeys(node, idx, element.first);
//
//        self().ctr().updatePageG(node);
//        LeafDispatcher::dispatch(node.page(), SetAndReindexFn(me()), idx, element);
//    }


    template <typename Node>
    Value getLeafDataFn(const Node* node, Int idx) const
    {
        return node->value(idx);
    }

    MEMORIA_CONST_FN_WRAPPER_RTN(GetLeafDataFn, getLeafDataFn, Value);

    Value getLeafData(const NodeBaseG& node, Int idx) const
    {
        return LeafDispatcher::dispatchConstRtn(node.page(), GetLeafDataFn(me()), idx);
    }




    template <typename Node>
    void setLeafDataFn(Node* node, Int idx, const Value& val) const
    {
        node->value(idx) = val;
    }

    MEMORIA_CONST_FN_WRAPPER(SetLeafDataFn, setLeafDataFn);



    void setLeafData(NodeBaseG& node, Int idx, const Value &val)
    {
    	self().updatePageG(node);
        LeafDispatcher::dispatch(node.page(), SetLeafDataFn(me()), idx, val);
    }

    MEMORIA_DECLARE_NODE_FN(LayoutNodeFn, layout);
    void layoutNode(NodeBaseG& node, UBigInt active_streams) const
    {
        NonLeafDispatcher::dispatch(node, LayoutNodeFn(), active_streams);
    }


    template <typename LeafElement>
    struct SetLeafEntryFn {

        template <Int Idx, typename Tree>
        void stream(Tree* tree, Int idx, const LeafElement& element, Accumulator* delta)
        {
            MEMORIA_ASSERT_TRUE(tree != nullptr);

            auto previous0 = tree->value(0, idx);
            auto previous1 = tree->value(1, idx);

            tree->value(0, idx) = element.first;
            tree->value(1, idx) = element.second;

            std::get<Idx>(*delta)[0] = element.first - previous0;
            std::get<Idx>(*delta)[1] = element.second - previous1;

            tree->reindex();
        }

        template <typename Node>
        void treeNode(Node* node, Int stream, Int idx, const LeafElement& element, Accumulator* delta)
        {
            node->template processStream<0>(*this, idx, element, delta);
        }
    };


    template <typename LeafElement>
    Accumulator setLeafEntry(NodeBaseG& node, Int stream, Int idx, const LeafElement& element) const
    {
        Accumulator delta;

        self().updatePageG(node);
        LeafDispatcher::dispatch(node.page(), SetLeafEntryFn<LeafElement>(), stream, idx, element, &delta);

        return delta;
    }


MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::vmap::CtrToolsName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS





#undef M_TYPE
#undef M_PARAMS


}


#endif
