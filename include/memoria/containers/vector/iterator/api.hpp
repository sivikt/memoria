
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_MODELS_ARRAY_ITERATOR_MODEL_API_HPP
#define _MEMORIA_MODELS_ARRAY_ITERATOR_MODEL_API_HPP

#include <iostream>

#include <memoria/containers/vector/names.hpp>
#include <memoria/core/container/iterator.hpp>

#include <memoria/core/tools/walkers.hpp>
#include <memoria/core/tools/sum_walker.hpp>

#include <memoria/core/tools/array_data.hpp>

namespace memoria    {

using namespace memoria::btree;


MEMORIA_ITERATOR_PART_BEGIN(memoria::models::array::IteratorContainerAPIName)

    typedef typename Base::NodeBase                                                 NodeBase;
    typedef typename Base::NodeBaseG                                                NodeBaseG;
    typedef typename Base::Container                                                Container;

    typedef typename Container::Key                                                 Key;

    typedef typename Base::Container::Page                                          PageType;
    typedef typename Base::Container::ID                                            ID;

    typedef typename Container::Types::Allocator                                    Allocator;
    typedef typename Container::Types::DataPage                                     DataPage;
    typedef typename Container::Types::DataPageG                                    DataPageG;
    typedef typename Container::Types::Buffer                                       Buffer;
    typedef typename Container::Types::BufferContentDescriptor                      BufferContentDescriptor;
    typedef typename Container::Types::CountData                                    CountData;
    typedef typename Container::Types::Pages::NodeDispatcher                        NodeDispatcher;
    typedef typename Container::Types::ElementType                        			ElementType;

    typedef IData<ElementType>														IDataType;

    typedef typename Base::TreePath                                                 TreePath;

    static const Int PAGE_SIZE = Base::Container::Allocator::PAGE_SIZE;


    MEMORIA_PUBLIC Int getElementSize() const
    {
        return me()->model().getElementSize();
    }


    MEMORIA_PUBLIC BigInt read(IDataType& data, BigInt start, BigInt length)
    {
        return me()->model().read(*me(), data, start, length);
    }

    MEMORIA_PUBLIC BigInt read(IDataType& data)
    {
        return read(data, 0, data.getSize());
    }

    
    void insert(const IDataType& data, BigInt start, BigInt length);
    void insert(const IDataType& data);

    MEMORIA_PUBLIC void insert(const ArrayData<ElementType>& data) {
        insert((IDataType&)data);
    }

    template <typename T>
    MEMORIA_PUBLIC void insert(const T& value)
    {
        me()->insert(ArrayData<ElementType>(value));
    }

    void update(const IDataType& data, BigInt start, BigInt length);
    void update(const IDataType& data);

    MEMORIA_PUBLIC void remove(BigInt length)
    {
        me()->model().removeDataBlock(*me(), length);
    }

    void remove(MyType& to)
    {
        me()->model().removeDataBlock(*me(), to);
    }

    BigInt skip(BigInt distance);
    BigInt skipFw(BigInt distance);
    BigInt skipBw(BigInt distance);

    MEMORIA_PUBLIC void dumpKeys(ostream& out)
    {
        Base::dumpKeys(out);

        out<<"Pos:     "<<me()->pos()<<endl;
        out<<"DataPos: "<<me()->dataPos()<<endl;
    }

    MEMORIA_PUBLIC void dumpPages(ostream& out)
    {
        Base::dumpPages(out);
        me()->model().dump(me()->data(), out);
    }

    MEMORIA_PUBLIC void dumpPath(ostream& out)
    {
        Base::dumpPath(out);

        if (me()->data().isSet())
        {
            out<<"Data:    "<<IDValue(me()->data()->id())<<" at "<<me()->path().data().parent_idx()<<endl;
        }
        else {
            out<<"No Data page is set"<<endl;
        }
    }


    MEMORIA_PUBLIC bool operator++()
    {
        Int size = me()->getElementSize();
        return me()->skip(size) = size;
    }

    MEMORIA_PUBLIC bool operator++(int)
    {
        return me()->skip(1) = 1;
    }

    MEMORIA_PUBLIC bool operator+=(Int count)
    {
        return me()->skip(count) = count;
    }

    MEMORIA_PUBLIC bool operator--()
    {
        return me()->skip(1);
    }

    MEMORIA_PUBLIC bool operator--(int)
    {
        return me()->skip(-1) = 1;
    }

    MEMORIA_PUBLIC bool operator-=(Int count)
    {
        return me()->skip(-count) = count;
    }


//    void assign(const IData& data)
//    {
//      update(data);
//    }

    template <typename T>
    MEMORIA_PUBLIC operator T()
    {
        T value;

        ArrayData<ElementType> data(value);

        me()->read(data);

        return value;
    }

MEMORIA_ITERATOR_PART_END


#define M_TYPE      MEMORIA_ITERATOR_TYPE(memoria::models::array::IteratorContainerAPIName)
#define M_PARAMS    MEMORIA_ITERATOR_TEMPLATE_PARAMS


MEMORIA_PUBLIC M_PARAMS
void M_TYPE::insert(const IDataType& data, BigInt start, BigInt length)
{
    me()->model().insertData(*me(), data, start, length);
}

MEMORIA_PUBLIC M_PARAMS
void M_TYPE::insert(const IDataType& data)
{
    me()->model().insertData(*me(), data);
}



MEMORIA_PUBLIC M_PARAMS
void M_TYPE::update(const IDataType& data, BigInt start, BigInt len)
{
    me()->model().updateData(*me(), data, start, len);
}

MEMORIA_PUBLIC M_PARAMS
void M_TYPE::update(const IDataType& data)
{
    me()->model().updateData(*me(), data, 0, data.getSize());
}



MEMORIA_PUBLIC M_PARAMS
BigInt M_TYPE::skip(BigInt distance)
{
    if (distance > 0)
    {
        return skipFw(distance);
    }
    else {
        return skipBw(-distance);
    }
}

MEMORIA_PUBLIC M_PARAMS
BigInt M_TYPE::skipFw(BigInt count)
{
    Int element_size = me()->getElementSize();

    BigInt distance = count * element_size;

    //FIXME: handle START properly
    if (me()->isNotEmpty())
    {
        Int     data_size   = me()->data()->size();
        Int     data_pos    = me()->dataPos();
        BigInt  pos         = me()->pos();

        if (distance + data_pos <= data_size)
        {
            // A trivial case when the offset is within current data page

            // we need to check for EOF if a data page
            // is the last one in the index node
            if (distance + data_pos == data_size)
            {
                if (me()->nextKey())
                {
                    // do nothing
                }
                else {
                    // Eof
                    me()->prevKey();
                    me()->dataPos() = me()->data()->size();
                }
            }
            else {
                me()->dataPos() += distance;
            }
        }
        else
        {
            SumTreeWalker<Container, Key, true> walker(distance + data_pos, me()->model());

            bool end = me()->model().walkFw(me()->path(), me()->key_idx(), walker);

            me()->model().finishPathStep(me()->path(), me()->key_idx());

            if (end)
            {
                me()->dataPos()     = me()->data()->size();

                me()->cache().setup(pos + (walker.sum() - data_pos) - me()->dataPos(), 0);

                return (walker.sum() - data_pos) / element_size;
            }
            else {

                me()->dataPos()     = walker.remainder();

                me()->cache().setup(pos + distance - me()->dataPos(), 0);
            }
        }

        //FIXME: return true distance
        return count;
    }
    else {
        return 0;
    }
}


MEMORIA_PUBLIC M_PARAMS
BigInt M_TYPE::skipBw(BigInt count)
{
    Int element_size = me()->getElementSize();

    BigInt distance = count * element_size;


    //FIXME: handle EOF properly
    if (me()->isNotEmpty())
    {
        BigInt pos = me()->pos();

        Int idx = me()->dataPos();

        if (distance <= idx)
        {
            // A trivial case when the offset is within current data page
            // we need to check for START if a data page
            // is the first in the index node
            me()->dataPos()     -= distance;
        }
        else
        {
            Int data_size   = me()->data()->size();
            Int to_add      = data_size - idx;
            SumTreeWalker<Container, Key, false> walker(distance + to_add, me()->model());

            //FIXME: does 'end' means the same as for StepFw()?
            bool end        = me()->model().walkBw(me()->path(), me()->key_idx(), walker);

            me()->model().finishPathStep(me()->path(), me()->key_idx());

            if (end)
            {
                me()->dataPos()     = 0;

                me()->cache().setup(0, 0);

                return (walker.sum() - to_add) / element_size;
            }
            else {
                me()->dataPos()     = me()->data()->size() - walker.remainder();

                me()->cache().setup((pos - distance) - me()->dataPos(), 0);
            }
        }

        return count;
    }
    else {
        return 0;
    }
}


#undef M_TYPE
#undef M_PARAMS


}



#endif
