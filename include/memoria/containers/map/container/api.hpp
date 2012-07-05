
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_MODELS_IDX_MAP_CONTAINER_API_HPP
#define	_MEMORIA_MODELS_IDX_MAP_CONTAINER_API_HPP


#include <memoria/prototypes/btree/pages/tools.hpp>
#include <memoria/containers/map/names.hpp>
#include <memoria/core/container/container.hpp>



namespace memoria    {

using namespace memoria::btree;

MEMORIA_CONTAINER_PART_BEGIN(memoria::models::idx_map::CtrApiName)

    typedef typename Base::Iterator                                             Iterator;
	typedef typename Base::Key                                             		Key;
	typedef typename Base::Value                                             	Value;
	typedef typename Base::Element                                             	Element;
	typedef typename Base::Accumulator                                          Accumulator;


    Iterator find(Key key)
    {
    	Iterator iter = me()->findLE(key, 0);

    	if (!iter.IsEnd())
    	{
    		if (key == iter.key())
    		{
    			return iter;
    		}
    		else {
    			return me()->End();
    		}
    	}
    	else {
    		return iter;
    	}
    }

    Iterator find1(Key key)
    {
    	Iterator iter = me()->findLE(key, 0);

    	if (!iter.IsEnd())
    	{
    		if (key == iter.key())
    		{
    			return iter;
    		}
    		else {
    			return me()->End();
    		}
    	}
    	else {
    		return iter;
    	}
    }


    Iterator operator[](Key key)
    {
    	Iterator iter = me()->findLE(key, 0);

    	if (iter.IsEnd() || key != iter.key())
    	{
    		Accumulator keys;
    		keys[0] = key;
    		me()->Insert(iter, keys);
    		iter.PrevKey();
    	}

    	return iter;
    }

    bool Remove(Key key)
    {
    	Iterator iter = me()->findLE(key, 0);

    	if (key == iter.key(0))
    	{
    		iter.Remove();
    		return true;
    	}
    	else {
    		return false;
    	}
    }

    void Remove(Iterator& from, Iterator& to)
    {
    	Accumulator keys;
    	me()->RemoveEntries(from, to, keys);

    	if (!to.IsEnd())
    	{
    		to.UpdateUp(keys);
    	}

    	to.cache().InitState();
    }

    void Insert(Iterator& iter, const Element& element)
    {
    	Accumulator delta = element.first - iter.prefixes();

    	Element e(delta, element.second);

    	if (Base::Insert(iter, e))
    	{
    		iter.UpdateUp(-delta);
    	}
    }

    void InsertRaw(Iterator& iter, const Element& element)
    {
    	Base::Insert(iter, element);
    }

    bool Contains(Key key)
    {
    	return !me()->find(key).IsEnd();
    }

    bool Contains1(Key key)
    {
    	return !me()->find1(key).IsEnd();
    }

    bool RemoveEntry(Iterator& iter, Accumulator& keys)
    {
    	bool result = Base::RemoveEntry(iter, keys);

    	if (!iter.IsEnd())
    	{
    		iter.UpdateUp(keys);
    	}

    	return result;
    }

MEMORIA_CONTAINER_PART_END

}


#endif
