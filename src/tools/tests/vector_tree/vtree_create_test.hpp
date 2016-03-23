
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

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

class VectorTreeCreateTest: public VectorTreeTestBase {

    using Base   = VectorTreeTestBase;
    using MyType = VectorTreeCreateTest;

    Int max_degree_ = 10;
    Int iterations_ = 1;

public:

    VectorTreeCreateTest(): VectorTreeTestBase("Create")
    {
        this->size_ = 100000;

        MEMORIA_ADD_TEST_PARAM(max_degree_);
        MEMORIA_ADD_TEST_PARAM(iterations_);

        MEMORIA_ADD_TEST(testFillTree);
    }

    virtual ~VectorTreeCreateTest() throw () {}

    void testFillTree()
    {
        auto snp = branch();

        auto tree = create<CtrName>(snp);

        TreeNode root = fillRandom(*tree.get(), size_, max_degree_);

        check(MA_SRC);

        checkTree(*tree.get(), root);

        commit();
    }
};

}}