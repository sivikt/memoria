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

#include <memoria/v1/memoria.hpp>

#include <memoria/v1/tools/tests.hpp>
#include <memoria/v1/tools/tools.hpp>

#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>
#include "multimap_test_base.hpp"

namespace memoria {
namespace v1 {

template <
    typename MapName
>
class MultiMapCreateTest: public MultiMapTestBase<MapName> {

    using MyType = MultiMapCreateTest<MapName>;
    using Base   = MultiMapTestBase<MapName>;


    using typename Base::IteratorPtr;
    using typename Base::Key;
    using typename Base::Value;
    using typename Base::Ctr;

    template <typename T>
    using TypeTag = typename Base::template TypeTag<T>;

    using Base::sizes_;

    using Base::commit;
    using Base::drop;
    using Base::branch;
    using Base::allocator;
    using Base::snapshot;
    using Base::check;
    using Base::getRandom;

    using Base::checkData;
    using Base::out;

    using Base::createRandomShapedMapData;
    using Base::make_key;
    using Base::make_value;

public:

    MultiMapCreateTest(StringRef name): Base(name)
    {
        MEMORIA_ADD_TEST_WITH_REPLAY(runCreateTest, replayCreateTest);
    }

    virtual ~MultiMapCreateTest() throw () {}

    void runCreateTest()
    {
        auto snp = branch();
        auto map = create<MapName>(snp);


        auto map_data = createRandomShapedMapData(
                sizes_[0],
                sizes_[1],
                [this](auto k) {return this->make_key(k, TypeTag<Key>());},
                [this](auto k, auto v) {return this->make_value(this->getRandom(), TypeTag<Value>());}
        );

        using EntryAdaptor = mmap::MMapAdaptor<Ctr>;

        auto iter = map->begin();

        EntryAdaptor stream_adaptor(map_data);
        auto totals = iter->bulk_insert(stream_adaptor);

        auto sizes = map->sizes();
        AssertEQ(MA_RAW_SRC, totals, sizes);

        checkData(*map.get(), map_data);

        snp->commit();
    }

    void replayCreateTest()
    {

    }


//    template <typename T> struct TypeTag {};
//
//    template <typename V, typename T>
//    T make_key(V&& num, TypeTag<T>) {
//        return num;
//    }
//
//    template <typename V>
//    String make_key(V&& num, TypeTag<String>)
//    {
//        stringstream ss;
//        ss<<"'";
//        ss.width(16);
//        ss << num;
//        ss<<"'";
//        return ss.str();
//    }
//
//    template <typename V>
//    UUID make_key(V&& num, TypeTag<UUID>)
//    {
//        return UUID(0, num);
//    }
//
//
//
//    template <typename V, typename T>
//    T make_value(V&& num, TypeTag<T>) {
//        return num;
//    }
//
//    template <typename V>
//    String make_value(V&& num, TypeTag<String>)
//    {
//        stringstream ss;
//        ss << num;
//        return ss.str();
//    }
//
//    template <typename V>
//    UUID make_value(V&& num, TypeTag<UUID>)
//    {
//        if (num != 0) {
//            return UUID::make_random();
//        }
//        else {
//            return UUID();
//        }
//    }
};

}}
