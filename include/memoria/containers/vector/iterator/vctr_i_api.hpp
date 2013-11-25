
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CONTAINER_vctr_ITERATOR_API_HPP
#define _MEMORIA_CONTAINER_vctr_ITERATOR_API_HPP

#include <memoria/core/types/types.hpp>
#include <memoria/core/tools/idata.hpp>
#include <memoria/core/tools/dump.hpp>

#include <memoria/containers/vector/vctr_names.hpp>
#include <memoria/containers/vector/vctr_tools.hpp>
#include <memoria/core/container/iterator.hpp>
#include <memoria/core/container/macros.hpp>

#include <iostream>

namespace memoria    {

MEMORIA_ITERATOR_PART_BEGIN(memoria::mvector::ItrApiName)

    typedef Ctr<typename Types::CtrTypes>                                       Container;


    typedef typename Base::Allocator                                            Allocator;
    typedef typename Base::NodeBase                                             NodeBase;
    typedef typename Base::NodeBaseG                                            NodeBaseG;

    typedef typename Container::Value                                           Value;
    typedef typename Container::Accumulator                                     Accumulator;

    typedef typename Container::DataSource                                      DataSource;
    typedef typename Container::DataTarget                                      DataTarget;
    typedef typename Container::LeafDispatcher                                  LeafDispatcher;
    typedef typename Container::Position                                        Position;

    typedef typename Container::Types::CtrSizeT									CtrSizeT;

    bool operator++() {
        return self().skipFw(1);
    }

    bool operator--() {
        return self().skipBw(1);
    }

    bool operator++(int) {
        return self().skipFw(1);
    }

    bool operator--(int) {
        return self().skipFw(1);
    }

    CtrSizeT operator+=(CtrSizeT size)
    {
        return self().skipFw(size);
    }

    CtrSizeT operator-=(CtrSizeT size)
    {
        return self().skipBw(size);
    }

    bool isEof() const {
        return self().idx() >= self().size();
    }

    bool isBof() const {
        return self().idx() < 0;
    }

//  bool nextLeaf()
//  {
//      auto& self      = this->self();
//      auto& ctr       = self.ctr();
//
//      auto next = ctr.getNextNodeP(self.leaf());
//
//      if (next)
//      {
//          self.leaf() = next;
//          self.idx()  = 0;
//          return true;
//      }
//      else {
//          return false;
//      }
//  }

    void insert(std::vector<Value>& data)
    {
        auto& self = this->self();
        auto& model = self.ctr();

        MemBuffer<Value> buf(data);

        model.insert(self, buf);

        model.markCtrUpdated();
    }

    void insert(Value data)
    {
        auto& self = this->self();
        auto& model = self.ctr();

        MemBuffer<Value> buf(&data, 1);

        model.insert(self, buf);

        model.markCtrUpdated();
    }

    Int size() const
    {
        return self().leafSize(0);
    }

    MEMORIA_DECLARE_NODE_FN(ReadFn, read);


    CtrSizeT read(DataTarget& data)
    {
        auto& self = this->self();
        mvector::VectorTarget target(&data);

        return self.ctr().readStream(self, target);
    }

    CtrSizeT read(std::vector<Value>& data)
    {
        MemBuffer<Value> buf(data);
        return read(buf);
    }

    Value value() const
    {
        Value data;
        MemBuffer<Value> buf(&data, 1);

        CtrSizeT length = read(buf);

        if (length == 1)
        {
            return data;
        }
        else if (length == 0)
        {
            throw Exception(MA_SRC, "Attempt to read vector after its end");
        }
        else {
            throw Exception(MA_SRC, "Invalid vector read");
        }
    }

    void remove(CtrSizeT size)
    {
        auto& self = this->self();
        self.ctr().remove(self, size);

        self.ctr().markCtrUpdated();
    }

    std::vector<Value> subVector(CtrSizeT size)
    {
        std::vector<Value> data(size);

        auto iter = self();

        auto readed = iter.read(data);

        MEMORIA_ASSERT(readed, ==, size);

        return data;
    }

    CtrSizeT skipFw(CtrSizeT amount);
    CtrSizeT skipBw(CtrSizeT amount);
    CtrSizeT skip(CtrSizeT amount);

    void seek(CtrSizeT pos)
    {
    	CtrSizeT current_pos = self().pos();
        self().skip(pos - current_pos);
    }

    struct PosFn {
        Accumulator prefix_;

        template <typename NodeTypes>
        void treeNode(const LeafNode<NodeTypes>* node, Int idx) {}

        template <typename NodeTypes>
        void treeNode(const BranchNode<NodeTypes>* node, Int idx)
        {
            node->sums(0, idx, prefix_);
        }
    };


    CtrSizeT pos() const
    {
        auto& self = this->self();

        PosFn fn;

        self.ctr().walkUp(self.leaf(), self.idx(), fn);

        return std::get<0>(fn.prefix_)[0] + self.key_idx();
    }

    CtrSizeT dataPos() const
    {
        return self().idx();
    }

    CtrSizeT prefix() const
    {
        return self().cache().prefix();
    }

    Accumulator prefixes() const {
        Accumulator acc;
        std::get<0>(acc)[0] = prefix();
        return acc;
    }

    void ComputePrefix(CtrSizeT& accum)
    {
        accum = prefix();
    }

    void ComputePrefix(Accumulator& accum)
    {
        accum = prefixes();
    }

MEMORIA_ITERATOR_PART_END

#define M_TYPE      MEMORIA_ITERATOR_TYPE(memoria::mvector::ItrApiName)
#define M_PARAMS    MEMORIA_ITERATOR_TEMPLATE_PARAMS

M_PARAMS
typename M_TYPE::CtrSizeT M_TYPE::skip(CtrSizeT amount)
{
    auto& self = this->self();

    if (amount > 0)
    {
        return self.skipFw(amount);
    }
    else if (amount < 0) {
        return self.skipBw(-amount);
    }
    else {
        return 0;
    }
}


M_PARAMS
typename M_TYPE::CtrSizeT M_TYPE::skipFw(CtrSizeT amount)
{
    return self().template _findFw<Types::template SkipForwardWalker>(0, amount);
}

M_PARAMS
typename M_TYPE::CtrSizeT M_TYPE::skipBw(CtrSizeT amount)
{
    return self().template _findBw<Types::template SkipBackwardWalker>(0, amount);
}




}

#undef M_TYPE
#undef M_PARAMS

#endif
