
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef __MEMORIA_PROTOTYPES_BTREE_ITERATOR_WALK_H
#define __MEMORIA_PROTOTYPES_BTREE_ITERATOR_WALK_H

#include <iostream>

#include <memoria/core/types/types.hpp>
#include <memoria/prototypes/btree/names.hpp>

namespace memoria    {

using namespace memoria::btree;


MEMORIA_ITERATOR_PART_BEGIN(memoria::btree::IteratorWalkName)

    typedef typename Base::NodeBase                                             NodeBase;
    typedef typename Base::Container::NodeDispatcher                                NodeDispatcher;

    template <typename Walker>
    class WalkHelperFn
    {
    	Int result_;
    	Walker& walker_;
    	Int idx_;
    public:
    	WalkHelperFn(Walker& walker, Int idx): walker_(walker), idx_(idx) {}
    	template <typename Node>
    	void operator()(Node* node)
    	{
    		result_ = walker_(node, idx_);
    	}
    	Int result() const {
    		return result_;
    	}
    };


    template <typename Walker>
    bool WalkFw(NodeBase*& index, Int &idx, Walker &walker)
    {
    	// Walk up
    	while (true)
    	{
    		WalkHelperFn<Walker> fn(walker, idx);
    		NodeDispatcher::Dispatch(index, fn);

    		if (fn.result() == -1)
    		{
    			if (!index->is_root())
    			{
    				// The case when index->parent_idx() == parent.size
    				// should be handled correctly in the walker
    				idx 	= index->parent_idx() + 1;
    				index 	= me_.GetParent(index);
    			}
    			else {
    				// EOF
    				idx 	= -1;
    				break;
    			}
    		}
    		else {
    			idx = fn.result();
    			break;
    		}
    	}

    	if (idx != -1)
    	{
    		// walk down
    		while (true)
    		{
    			WalkHelperFn<Walker> fn(walker, idx);
    			NodeDispatcher::Dispatch(index, fn);

    			if (fn.result() == -1)
    			{
    				// this should not happened here
    				throw MemoriaException(MEMORIA_SOURCE, "WalkFw:Down: idx == -1");
    			}

    			if (!index->is_leaf())
    			{
    				index = me_.GetChild(index, fn.result());
    				idx = 0; // FIXME: check this
    			}
    			else {
    				idx = fn.result();
    				return false;
    			}
    		}
    	}
    	else {
    		// END
    		while (true)
    		{
    			if (!index->is_leaf())
    			{
    				index = me_.model().GetLastChild(index);
    			}
    			else {
    				//FIXME: remove '-1'
    				idx = me_.model().GetChildrenCount(index) - 1;
    				return true;
    			}
    		}
    	}

    }


    template <typename Walker>
    bool WalkBw(NodeBase*& index, Int &idx, Walker &walker)
    {
        // Walk up
        while (true)
        {
        	WalkHelperFn<Walker> fn(walker, idx);
        	NodeDispatcher::Dispatch(index, fn);

        	if (fn.result() == -1)
        	{
        		if (!index->is_root())
        		{
        			idx 	= index->parent_idx() - 1;
        			index 	= me_.GetParent(index);
        		}
        		else {
        			// START
        			idx 	= -1;
        			break;
        		}
        	}
        	else {
        		idx = fn.result();
        		break;
        	}
        }

        if (idx != -1)
        {
        	// walk down
        	while (true)
        	{
        		WalkHelperFn<Walker> fn(walker, idx);
        		NodeDispatcher::Dispatch(index, fn);

        		if (!index->is_leaf())
        		{
        			index = me_.GetChild(index, fn.result());
        			idx = me_.model().GetChildrenCount(index) - 1;
        		}
        		else {
        			idx = fn.result();
        			return false;
        		}
        	}
        }
        else {
        	// START
        	idx = 0;
        	while (true)
        	{
        		if (!index->is_leaf())
        		{
        			index = me_.GetChild(index, 0);
        		}
        		else {
        			return true;
        		}
        	}
        }
    }
    
    template <typename Walker>
    void walk_to_the_root(NodeBase* node, Int idx, Walker& walker)
    {
    	while(!node->is_root())
    	{
    		walker(node, idx);
    		idx  = node->parent_idx();
    		node = me_.GetParent(node);
    	}
    	walker(node, idx);
    }

MEMORIA_ITERATOR_PART_END

}

#endif
