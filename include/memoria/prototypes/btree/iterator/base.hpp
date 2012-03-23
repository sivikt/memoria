
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef __MEMORIA_PROTOTYPES_BTREE_ITERATOR_BASE_H
#define __MEMORIA_PROTOTYPES_BTREE_ITERATOR_BASE_H

#include <iostream>

#include <memoria/core/types/types.hpp>
#include <memoria/prototypes/btree/names.hpp>
#include <memoria/prototypes/btree/macros.hpp>

#include <memoria/core/tools/hash.hpp>

namespace memoria    {

using namespace memoria::btree;


MEMORIA_BTREE_ITERATOR_BASE_CLASS_NO_CTOR_BEGIN(BTreeIteratorBase)
public:
	typedef typename Base::Container::TreePath                                        	TreePath;
	typedef typename Base::Container::TreePath::Element                               	TreePathItem;
    typedef typename Base::Container::NodeBase                                        	NodeBase;
    typedef typename Base::Container::NodeBaseG                                       	NodeBaseG;
    typedef typename Base::Container::Allocator										  	Allocator;

    static const Int Indexes															= Base::Container::Indexes;

private:

    TreePath           	path_;
    Int                 key_idx_;
    BigInt				key_num_;

public:
    BTreeIteratorBase(): Base(), path_(), key_idx_(0), key_num_(0) {}

    BTreeIteratorBase(ThisType&& other): Base(std::move(other)), path_(std::move(other.path_)), key_idx_(other.key_idx_), key_num_(other.key_num_) {}

    BTreeIteratorBase(const ThisType& other): Base(other), path_(other.path_), key_idx_(other.key_idx_), key_num_(other.key_num_) {}

    void operator=(ThisType&& other)
    {
        path_       = other.path_;
        key_idx_    = other.key_idx_;
        key_num_	= other.key_num_;

        Base::operator=(std::move(other));
    }

    void operator=(const ThisType& other)
    {
    	path_       = other.path_;
    	key_idx_    = other.key_idx_;
    	key_num_	= other.key_num_;

    	Base::operator=(other);
    }

    bool operator==(const MyType& other) const
    {
    	return page() == other.page() && key_idx_ == other.key_idx_ && Base::operator==(other);
    }

    Int BuildHash() const
    {
    	const NodeBase* page0;

    	if (path_.GetSize() > 0)
    	{
    		page0 = path_[0].node().is_set() ? path_[0].node().page() : NULL;
    	}
    	else {
    		page0 = NULL;
    	}

    	return Base::BuildHash() ^ PtrToHash<const NodeBase*>::hash(page0) ^ key_idx_;
    }

    void SetNode(NodeBaseG& node, Int parent_idx)
    {
    	path_[node->level()].node() 		= node;
    	path_[node->level()].parent_idx() 	= parent_idx;
    }

    Int &key_idx()
    {
        return key_idx_;
    }

    const Int key_idx() const
    {
        return key_idx_;
    }

    NodeBaseG& page()
    {
        return path_[0];
    }

    const NodeBaseG& page() const
    {
    	return path_[0];
    }

    TreePathItem& leaf()
    {
    	return path_[0];
    }

    const TreePathItem& leaf() const
    {
    	return path_[0];
    }

    TreePath& path()
    {
    	return path_;
    }

    const TreePath& path() const
    {
    	return path_;
    }


    bool IsBegin()
    {
    	return key_idx() < 0;
    }

    bool IsEnd()
    {
    	return page().is_set() ? key_idx() >= page()->children_count() : true;
    }

    bool IsNotEnd()
    {
    	return !IsEnd();
    }

    bool IsEmpty()
    {
    	return page().is_empty() || page()->children_count() == 0;
    }

    bool IsNotEmpty()
    {
    	return !IsEmpty();
    }

    BigInt KeyNum() const
    {
    	return key_num_;
    }

    BigInt& KeyNum()
    {
    	return key_num_;
    }


    void Dump(ostream& out = cout, const char* header = NULL)
    {
    	out<<(header != NULL ? header : me()->GetDumpHeader())<<endl;

    	me()->DumpKeys(out);

    	me()->DumpBeforePath(out);
    	me()->DumpPath(out);

    	me()->DumpBeforePages(out);
    	me()->DumpPages(out);
    }

    String GetDumpHeader()
    {
    	return String(me()->model().type_name()) + " Iterator State";
    }

    void DumpPath(ostream& out)
    {
    	out<<"Path:"<<endl;

    	TreePath& path0 = me()->path();
    	for (int c = me()->path().GetSize() - 1; c >= 0; c--)
    	{
    		out<<"Node("<<c<<"): "<<IDValue(path0[c]->id())<<" idx="<<(c > 0 ? ToString(path0[c - 1].parent_idx()) : "")<<endl;
    	}
    }

    void DumpKeys(ostream& out)
    {
    	out<<"KeyIdx:  "<<me()->key_idx()<<endl;
    }

    void DumpBeforePath(ostream& out){}
    void DumpBeforePages(ostream& out){}

    void DumpPages(ostream& out)
    {
    	me()->model().Dump(me()->leaf().node(), out);
    }


MEMORIA_BTREE_ITERATOR_BASE_CLASS_END

} //memoria



#endif
