
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_MODELS_IDX_MAP_MODEL_TOOLS_HPP
#define	_MEMORIA_MODELS_IDX_MAP_MODEL_TOOLS_HPP


#include <memoria/prototypes/itree/names.hpp>
#include <memoria/core/container/container.hpp>



namespace memoria    {

MEMORIA_CONTAINER_PART_BEGIN(memoria::itree::ToolsName)

    typedef typename Base::Types                                                Types;
    typedef typename Base::Allocator                                              Allocator;

    typedef typename Allocator::Page                                              Page;
    typedef typename Page::ID                                                   ID;
    typedef typename Allocator::Transaction                                       Transaction;

    typedef typename Types::NodeBase                                            NodeBase;
    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Types::Counters                                            Counters;
    typedef typename Base::Iterator                                             Iterator;

    typedef typename Types::Pages::NodeDispatcher                               NodeDispatcher;
    typedef typename Types::Pages::RootDispatcher                               RootDispatcher;
    typedef typename Types::Pages::LeafDispatcher                               LeafDispatcher;
    typedef typename Types::Pages::NonLeafDispatcher                            NonLeafDispatcher;

    typedef typename Types::Pages::Node2RootMap                                 Node2RootMap;
    typedef typename Types::Pages::Root2NodeMap                                 Root2NodeMap;

    typedef typename Types::Pages::NodeFactory                                  NodeFactory;

    typedef typename Base::Key                                                  Key;
    typedef typename Base::Value                                                Value;

    static const Int Indexes                                                    = Types::Indexes;
    static const Int MapType                                                    = Types::MapType;
    
    struct SetAndReindexFn1 {
        Int             i_;
        const Key*      keys_;
        const Value&    data_;
        bool            fixed_;
    public:
        SetAndReindexFn1(Int i, const Key *keys, const Value& data): i_(i), keys_(keys), data_(data), fixed_(true) {}

        template <typename T>
        void operator()(T *node)
        {
            for (Int c = 0; c < Indexes; c++) {
                node->map().key(c, i_) = keys_[c];
            }

            node->map().data(i_) = data_;

            if (i_ < node->map().size() - 1) {
                for (Int c = 0; c < Indexes; c++) {
                    node->map().key(c, i_ + 1) -= keys_[c];
                }
            }
            else {
                fixed_ = false;
            }

            node->map().Reindex();
        }
    };


    struct SetAndReindexFn2 {
        Int             i_;
        const Key*      keys_;

    public:
        SetAndReindexFn2(Int i, const Key *keys): i_(i), keys_(keys) {}

        template <typename T>
        void operator()(T *node)
        {
            for (Int c = 0; c < Indexes; c++) {
                node->map().key(c, i_) -= keys_[c];
            }

            node->map().Reindex();
        }
    };



    void SetLeafDataAndReindex(NodeBaseG& node, Int idx, const Key *keys, const Value &val);

    template <typename Node>
    bool CheckNodeContent(Node *node);


    template <typename Node1, typename Node2>
    bool CheckNodeWithParentContent(Node1 *node, Node2 *parent);

    //
//    Iterator GetByIndex(BigInt index)
//    {
//        Iterator i = Base::GetByIndex(index);
//        i.ComputeBase();
//        return i;
//    }

MEMORIA_CONTAINER_PART_END



#define M_TYPE 		MEMORIA_CONTAINER_TYPE(memoria::itree::ToolsName)
#define M_PARAMS 	MEMORIA_CONTAINER_TEMPLATE_PARAMS

M_PARAMS
void M_TYPE::SetLeafDataAndReindex(NodeBaseG& node, Int idx, const Key *keys, const Value &val)
{
	node.update();
	SetAndReindexFn1 fn1(idx, keys, val);
	LeafDispatcher::Dispatch(node, fn1);

	if (!fn1.fixed_)
	{
		Iterator i(node, idx, *me());
		if (i.NextKey())
		{
			SetAndReindexFn2 fn2(i.key_idx(), keys);
			LeafDispatcher::Dispatch(i.page(), fn2);

			if (MapType == MapTypes::Index)
			{
				me()->UpdateBTreeKeys(i.page());
			}
			else if (i.key_idx() >= me()->GetChildrenCount(i.page()) -1)
			{
				me()->UpdateBTreeKeys(i.page());
			}
		}
	}
}

M_PARAMS
template <typename Node>
bool M_TYPE::CheckNodeContent(Node *node) {
	bool errors = false;

	for (Int i = 0; i < Indexes; i++) {
		Key key = 0;

		for (Int c = 0; c < node->map().size(); c++) {
			key += node->map().key(i, c);
		}

		if (key != node->map().max_key(i)) {
			MEMORIA_ERROR(me(), "Sum of keys doen't match max_key for key", i, key, node->map().max_key(i));
			errors = true;
		}
	}

	return errors;
}

M_PARAMS
template <typename Node1, typename Node2>
bool M_TYPE::CheckNodeWithParentContent(Node1 *node, Node2 *parent)
{
	bool errors = false;
	for (Int c = 0; c < Indexes; c++)
	{
		if (node->map().max_key(c) != parent->map().key(c, node->parent_idx()))
		{
			MEMORIA_ERROR(me(), "Invalid parent-child nodes chain", c, node->map().max_key(c), parent->map().key(c, node->parent_idx()), "for", node->id(), parent->id(), node->parent_idx());
			errors = true;
		}
	}

	errors = CheckNodeContent(node) || errors;

	return errors;
}


#undef M_TYPE
#undef M_PARAMS

}

#endif
