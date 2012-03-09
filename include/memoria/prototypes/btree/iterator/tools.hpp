
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef __MEMORIA_PROTOTYPES_BTREE_ITERATOR_TOOLS_H
#define __MEMORIA_PROTOTYPES_BTREE_ITERATOR_TOOLS_H

#include <iostream>

#include <memoria/core/types/types.hpp>
#include <memoria/prototypes/btree/names.hpp>

namespace memoria    {

using namespace memoria::btree;

MEMORIA_ITERATOR_PART_BEGIN(memoria::btree::IteratorToolsName)


    typedef typename Base::Container                                                Container;
	typedef typename Container::Accumulator                                  		Accumulator;
	typedef typename Container::Types::NodeBase                                     NodeBase;
	typedef typename Container::Types::NodeBaseG                                    NodeBaseG;

    typedef typename Container::Key                                                 Key;
    typedef typename Container::Value                                               Value;



    Value GetData() const
    {
        return me()->model().GetLeafData(me()->page(), me()->key_idx());
    }

    Key GetKey(Int keyNum) const
    {
        return Container::GetKey(me()->page(), keyNum, me()->key_idx());
    }

    Accumulator GetKeys() const
    {
    	Accumulator accum;
    	Container::GetKeys(me()->page(), me()->key_idx(), accum);
    	return accum;
    }

    void Dump(ostream& out = cout)
    {
    	out<<"SumSet Iterator state"<<endl;
    	out<<"KeyIdx: 	   "<<me()->key_idx()<<endl;

    	me()->model().Dump(me()->page(), out);
    }


MEMORIA_ITERATOR_PART_END

}

#endif
