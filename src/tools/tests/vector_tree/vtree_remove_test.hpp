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

#include <memoria/v1/memoria.hpp>

#include <memoria/v1/tools/tests.hpp>
#include <memoria/v1/tools/tools.hpp>

#include "vtree_test_base.hpp"

#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>

namespace memoria {
namespace v1 {

using namespace v1::louds;

class VectorTreeRemoveTest: public VectorTreeTestBase {

    using Base      = VectorTreeTestBase;
    using MyType    = VectorTreeRemoveTest;

    Int     iterations_     = 1000;
    Int     max_degree_     = 10;
    Int     remove_batch_   = 100;

public:

    VectorTreeRemoveTest(): Base("Remove")
    {
        size_ = 20000;

        MEMORIA_ADD_TEST_PARAM(iterations_);
        MEMORIA_ADD_TEST_PARAM(max_degree_);
        MEMORIA_ADD_TEST_PARAM(remove_batch_);

        MEMORIA_ADD_TEST(testRemoveNodes);
    }

    virtual ~VectorTreeRemoveTest() throw () {}


    void removeNodes(Ctr& tree, TreeNode& tree_node, Int max_size)
    {
        Int size = 0;
        LoudsNode root = tree.seek(0)->node();

        removeNode(tree, root, tree_node, size, max_size);
    }

    bool removeNode(Ctr& tree, const LoudsNode& node, TreeNode& tree_node, Int& size, Int max_size)
    {
        if (size < max_size)
        {
            for (Int c = 0; c < tree_node.children();)
            {
                LoudsNode child = tree.child(node, c)->node();

                if (removeNode(tree, child, tree_node.child(c), size, max_size))
                {
                    tree_node.removeChild(c);
                }
                else {
                    c++;
                }
            }

            if (tree_node.children() == 0)
            {
                if (getRandom(2) && tree.nodes() > 1)
                {
                    tree.removeLeaf(node);
                    size++;

                    return true;
                }
            }
        }

        return false;
    }


    void testRemoveNodes()
    {
        auto snp = branch();

        auto tree = create<CtrName>(snp);

        TreeNode root = fillRandom(*tree.get(), size_, max_degree_);

        commit();

        auto ctr_name = tree->name();

        BigInt tree_nodes = tree->nodes();

        tree.reset();

        for (Int c = 0; c < iterations_ && tree_nodes; c++)
        {
            out()<<c<<std::endl;

            snp = branch();

            tree = find<CtrName>(snp, ctr_name);

            removeNodes(*tree.get(), root, remove_batch_);

            check(MA_SRC);

            checkTree(*tree.get(), root);
        }
    }
};

}}