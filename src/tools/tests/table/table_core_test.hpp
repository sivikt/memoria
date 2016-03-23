// Copyright 2015 Victor Smirnov
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

#include "../prototype/bttl/bttl_test_base.hpp"

#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>

namespace memoria {
namespace v1 {

template <
    typename CtrName,
    typename AllocatorT     = SmallInMemAllocator,
    typename ProfileT       = DefaultProfile<>
>
class TableCoreTest: public BTTLTestBase<CtrName, AllocatorT, ProfileT> {

    using Base   = BTTLTestBase<CtrName, AllocatorT, ProfileT>;
    using MyType = TableCoreTest<CtrName, AllocatorT, ProfileT>;

    using Allocator     = typename Base::Allocator;
    using AllocatorSPtr = typename Base::AllocatorSPtr;
    using CtrT          = typename Base::Ctr;

public:

    TableCoreTest(String name): Base(name)
    {
        MEMORIA_ADD_TEST(basicTest);
    }

    virtual ~TableCoreTest() throw () {}


    virtual void createAllocator(AllocatorSPtr& allocator)
    {
        allocator = std::make_shared<Allocator>();
    }

    void basicTest() {

    }

};

}}