
// Copyright 2013 Victor Smirnov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#pragma once

#include <memoria/prototypes/bt/bt_walkers.hpp>

#include <memoria/core/packed/sseq/packed_fse_searchable_seq.hpp>

namespace memoria {
namespace louds {


template <typename Types>
class FindEdgeWalkerBase {
protected:
    typedef Iter<typename Types::IterTypes> Iterator;
    typedef Ctr<typename Types::CtrTypes>   Container;

    typedef typename Types::BranchNodeEntry     BranchNodeEntry;

    WalkDirection direction_;

public:
    FindEdgeWalkerBase() {}

    WalkDirection& direction() {
        return direction_;
    }

    void empty(Iterator& iter)
    {
//      iter.iter_cache().setup(BranchNodeEntry());
    }
};



template <typename Types>
class FindEndWalker: public FindEdgeWalkerBase<Types> {

    typedef FindEdgeWalkerBase<Types>       Base;
    typedef typename Base::Iterator         Iterator;
    typedef typename Base::Container        Container;

    typedef typename Types::BranchNodeEntry     BranchNodeEntry;

    BranchNodeEntry prefix_;

public:
    typedef int32_t ReturnType;

    FindEndWalker(int32_t stream, const Container&) {}

    template <typename Node>
    ReturnType treeNode(const Node* node, int32_t start)
    {
        if (node->level() > 0)
        {
            VectorAdd(prefix_, node->maxKeys() - node->keysAt(node->children_count() - 1));
        }
        else {
            VectorAdd(prefix_, node->maxKeys());
        }

        return node->children_count() - 1;
    }

    void finish(Iterator& iter, int32_t idx)
    {
        iter.local_pos() = idx + 1;
        iter.iter_cache().setup(prefix_);
    }
};


template <typename Types>
class FindREndWalker: public FindEdgeWalkerBase<Types> {

    typedef FindEdgeWalkerBase<Types>       Base;
    typedef typename Base::Iterator         Iterator;
    typedef typename Base::Container        Container;
    typedef typename Types::BranchNodeEntry     BranchNodeEntry;

public:
    typedef int32_t ReturnType;

    FindREndWalker(int32_t stream, const Container&) {}

    template <typename Node>
    ReturnType treeNode(const Node* node)
    {
        return 0;
    }

    void finish(Iterator& iter, int32_t idx)
    {
        iter.local_pos() = idx - 1;

        iter.iter_cache().setup(BranchNodeEntry());
    }
};



template <typename Types>
class FindBeginWalker: public FindEdgeWalkerBase<Types> {

    typedef FindEdgeWalkerBase<Types>       Base;
    typedef typename Base::Iterator         Iterator;
    typedef typename Base::Container        Container;
    typedef typename Types::BranchNodeEntry     BranchNodeEntry;

public:
    typedef int32_t ReturnType;


    FindBeginWalker(int32_t stream, const Container&) {}


    template <typename Node>
    ReturnType treeNode(const Node* node, int32_t)
    {
        return 0;
    }

    void finish(Iterator& iter, int32_t idx)
    {
        iter.local_pos() = 0;
    }
};

template <typename Types>
class FindRBeginWalker: public FindEdgeWalkerBase<Types> {

    typedef FindEdgeWalkerBase<Types>       Base;
    typedef typename Base::Iterator         Iterator;
    typedef typename Base::Container        Container;

    typedef typename Types::BranchNodeEntry     BranchNodeEntry;

    BranchNodeEntry prefix_;

public:
    FindRBeginWalker(int32_t stream, const Container&) {}

    typedef int32_t ReturnType;



    template <typename Node>
    ReturnType treeNode(const Node* node)
    {
        VectorAdd(prefix_, node->maxKeys() - node->keysAt(node->children_count() - 1));

        return node->children_count() - 1;
    }

    void finish(Iterator& iter, int32_t idx)
    {
        iter.local_pos() = idx;

        iter.iter_cache().setup(prefix_);
    }
};





}}