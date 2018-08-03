
// Copyright 2016 Victor Smirnov
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



#include "packed_maxtree_test_base.hpp"

namespace memoria {
namespace v1 {
namespace tests {

template <typename PackedTreeT>
class PackedMaxTreeFindTest: public PackedMaxTreeTestBase<PackedTreeT> {

    using MyType = PackedMaxTreeFindTest<PackedTreeT>;
    using Base   = PackedMaxTreeTestBase<PackedTreeT>;

    typedef typename Base::Tree                                                 Tree;
    typedef typename Base::Values                                               Values;
    typedef typename Base::Value                                                Value;
    typedef typename Tree::IndexValue                                           IndexValue;

    using typename Base::TreePtr;

    using Base::createEmptyTree;
    using Base::fillVector;
    using Base::fillRandom;
    using Base::assertIndexCorrect;
    using Base::assertEqual;
    using Base::getRandom;
    using Base::createRandomValuesVector;
    using Base::assertEmpty;
    using Base::out;

public:

    int32_t iterations_ = 1000;

    PackedMaxTreeFindTest() {
        this->size_ = 16384;
    }

    MMA1_STATE_FILEDS(iterations_);

    static void init_suite(TestSuite& suite)
    {
        MMA1_CLASS_TESTS(suite, testFindForward);
    }

    template <typename Walker>
    auto find_fw(const TreePtr& tree, int32_t block, IndexValue limit)
    {
        int32_t end = tree->size();

        Walker walker(limit);

        for (int32_t c = 0; c < end; c++)
        {
            Value value = tree->value(block, c);

            if (walker.compare(value))
            {
                return walker.idx(c);
            }
            else {
                walker.next();
            }
        }

        return walker.idx(end);
    }

    void testFindForward()
    {
        for (int32_t c = 4; c < 1024; c *= 2){
            testFindForward(c);
        }

        for (int32_t c = 1024*3; c <= this->size_; c += 1024){
            testFindForward(c);
        }
    }


    void testFindForward(int32_t tree_size)
    {
        out() << tree_size << std::endl;

        auto tree = createEmptyTree();
        auto values = fillRandom(tree, tree_size);

        int32_t size = tree->size();

        for (int32_t c = 0; c < iterations_; c++)
        {
            int32_t end     = getRandom(size);
            int32_t block   = getRandom(Tree::Blocks);

            auto max     = tree->value(block, end);

            auto result1_lt = tree->findGTForward(block, max);
            auto result1_le = tree->findGEForward(block, max);

            auto result2_lt = find_fw<typename Tree::FindGTWalker>(tree, block, max);
            auto result2_le = find_fw<typename Tree::FindGEWalker>(tree, block, max);

            assert_equals(result1_lt.local_pos(), result2_lt.local_pos(), u"IDX {} {}", max, block);
            assert_equals(result1_le.local_pos(), result2_le.local_pos(), u"IDX {} {}", max, block);
        }
    }
};


}}}
