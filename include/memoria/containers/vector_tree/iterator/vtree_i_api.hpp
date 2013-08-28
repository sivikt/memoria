
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef MEMORIA_CONTAINERS_VTREE_I_API_HPP
#define MEMORIA_CONTAINERS_VTREE_I_API_HPP

#include <memoria/core/container/container.hpp>

#include <memoria/containers/vector_tree/vtree_names.hpp>

#include <functional>

namespace memoria {


MEMORIA_ITERATOR_PART_BEGIN(memoria::vtree::ItrApiName)

    typedef Ctr<VTreeCtrTypes<Types>>                           ContainerType;

    typedef typename ContainerType::Tree::Iterator              TreeIterator;
    typedef typename ContainerType::Vec::Iterator               VectorIterator;
    typedef typename ContainerType::Vec::Value                  Value;

    BigInt next_siblings() const
    {
        return self().tree_iter().next_siblings();
    }

    BigInt prev_siblings() const
    {
        return self().tree_iter().prev_siblings();
    }

    bool next_sibling()
    {
        auto& self = this->self();

        if (self.tree_iter().next_sibling())
        {
            BigInt data_base = self.tree_iter().template sumLabel<1>();
            self.vector_iter() = self.ctr().vector().seek(data_base);

            return true;
        }
        else {
            return false;
        }
    }

    bool prev_sibling()
    {
        auto& self = this->self();

        if (self.tree_iter().prev_sibling())
        {
            BigInt data_base = self.tree_iter().template sumLabel<1>();
            self.vector_iter() = self.ctr().vector().seek(data_base);

            return true;
        }
        else {
            return false;
        }
    }

    LoudsNode node() const
    {
        return self().tree_iter().node();
    }

    Value value() const
    {
        return self().vector_iter().value();
    }

    void insert(Value value)
    {
        self().vector_iter().insert(value);
        self().tree_iter().addLabel(1, 1);
    }

    void insert(const std::vector<Value>& values)
    {
        auto& self = this->self();

        std::vector<Value> copy = values;

        self.vector_iter().insert(copy);

        self.tree_iter().addLabel(1, values.size());
    }

    void insert(std::vector<Value>& values)
    {
        self().vector_iter().insert(values);
        self().tree_iter().addLabel(1, values.size());
    }

    BigInt lobBase() const
    {
        return self().tree_iter().template sumLabel<1>();
    }

    BigInt lobSize() const
    {
        return std::get<1>(self().tree_iter().labels());
    }

    void removeLob()
    {
        auto& self = this->self();

        BigInt data_base    = self.lobBase();
        BigInt data_size    = self.lobSize();

        self.vector_iter().seek(data_base);
        self.vector_iter().remove(data_size);
    }

    void removeLob(BigInt size)
    {
        auto& self = this->self();

        BigInt data_base    = self.lobBase();
        BigInt data_size    = self.lobSize();
        BigInt data_pos     = self.vector_iter().idx();

        if (data_pos + size < data_base + data_size)
        {
            return self.vector_iter().remove(size);
        }
        else {
            return self.vector_iter().remove(data_base + data_size - data_pos);
        }
    }

    BigInt skipFw(BigInt delta)
    {
        auto& self = this->self();

        BigInt data_base    = self.lobBase();
        BigInt data_size    = self.lobSize();
        BigInt data_pos     = self.vector_iter().idx();

        if (data_pos + delta < data_base + data_size)
        {
            return self.vector_iter().skipFw(delta);
        }
        else {
            return self.vector_iter().skipFw(data_base + data_size - data_pos);
        }
    }

    std::vector<Value> read() {
        BigInt data_size = self().lobSize();
        return self().vector_iter().subVector(data_size);
    }

MEMORIA_ITERATOR_PART_END

}


#endif
