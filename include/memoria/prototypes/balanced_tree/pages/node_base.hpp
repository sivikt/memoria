
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_BALANCEDTREE_PAGES_NODE_BASE_HPP
#define _MEMORIA_PROTOTYPES_BALANCEDTREE_PAGES_NODE_BASE_HPP

#include <memoria/core/tools/reflection.hpp>
#include <memoria/core/container/page.hpp>
#include <memoria/core/container/logs.hpp>

#include <memoria/core/types/typehash.hpp>

namespace memoria    	{
namespace balanced_tree {

using memoria::BitBuffer;



template <typename Base_>
class TreePage: public Base_ {
public:
    static const UInt VERSION = 1;

private:

    Int root_;
    Int leaf_;
    Int bitmap_;
    Int level_;

    Int size_;

public:



    enum {
        LEAF          = 0,
        ROOT          = 1,
        BITMAP        = 2
    }                                       	FLAGS;

    typedef Base_                               Base;
    typedef TreePage<Base>                      Me;
    typedef Me 									BasePageType;

    typedef typename Base::ID                   ID;


//    typedef typename MergeLists<
//                typename Base::FieldsList,
//                ConstValue<UInt, VERSION>,
//                decltype(root_),
//                decltype(leaf_),
//                decltype(bitmap_),
//                decltype(level_),
//                decltype(size_)
//    >::Result                                                                   FieldsList;

    TreePage(): Base() {}

    inline bool is_root() const {
        return root_;
    }

    void set_root(bool root) {
        root_ = root;
    }

    inline bool is_leaf() const {
        return leaf_;
    }

    void set_leaf(bool leaf) {
        leaf_ = leaf;
    }

    inline bool isBitmap() const {
        return bitmap_;
    }

    void setBitmap(bool bitmap) {
        bitmap_ = bitmap;
    }

    Int size() const
    {
        return size_;
    }

    Int children_count() const
    {
        return size_;
    }

    const Int& level() const
    {
    	return level_;
    }

    Int& level()
    {
    	return level_;
    }

protected:
    Int& map_size()
    {
        return size_;
    }
public:

    void generateDataEvents(IPageDataEventHandler* handler) const
    {
        Base::generateDataEvents(handler);

        handler->value("ROOT", 		&root_);
        handler->value("LEAF", 		&leaf_);
        handler->value("BITMAP",	&bitmap_);
        handler->value("SIZE", 		&size_);
        handler->value("LEVEL", 	&level_);
    }

    template <template <typename> class FieldFactory>
    void serialize(SerializationData& buf) const
    {
        Base::template serialize<FieldFactory>(buf);

        FieldFactory<Int>::serialize(buf, root_);
        FieldFactory<Int>::serialize(buf, leaf_);
        FieldFactory<Int>::serialize(buf, bitmap_);
        FieldFactory<Int>::serialize(buf, level_);

        FieldFactory<Int>::serialize(buf, size_);
    }

    template <template <typename> class FieldFactory>
    void deserialize(DeserializationData& buf)
    {
        Base::template deserialize<FieldFactory>(buf);

        FieldFactory<Int>::deserialize(buf, root_);
        FieldFactory<Int>::deserialize(buf, leaf_);
        FieldFactory<Int>::deserialize(buf, bitmap_);
        FieldFactory<Int>::deserialize(buf, level_);

        FieldFactory<Int>::deserialize(buf, size_);
    }


    template <typename PageType>
    void copyFrom(const PageType* page)
    {
        Base::copyFrom(page);

        this->set_root(page->is_root());
        this->set_leaf(page->is_leaf());
        this->setBitmap(page->isBitmap());

        this->level() = page->level();
    }
};







template <
        typename BaseType0
>
class NodePageBase: public BaseType0
{


public:
    typedef BaseType0                                                            Base;
    typedef BaseType0                                                            BasePageType;

    NodePageBase(): BaseType0()
    {
        init();
    }

    void init()
    {
        Base::init();
    }
};


}

template <typename Base>
struct TypeHash<balanced_tree::TreePage<Base>> {
    static const UInt Value = HashHelper<
    		TypeHash<Base>::Value,
    		balanced_tree::TreePage<Base>::VERSION,
    		TypeHash<Int>::Value,
    		TypeHash<Int>::Value,
    		TypeHash<Int>::Value,
    		TypeHash<Int>::Value,
    		TypeHash<Int>::Value
    >::Value;
};

template <typename Base>
struct TypeHash<balanced_tree::NodePageBase<Base>> {
    static const UInt Value = TypeHash<Base>::Value;
};


}






#endif
