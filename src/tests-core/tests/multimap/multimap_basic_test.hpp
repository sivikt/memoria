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

#include <memoria/v1/tests/tests.hpp>
#include <memoria/v1/tests/tools.hpp>

#include "multimap_test_base.hpp"

#include <vector>
#include <algorithm>
#include <sstream>
#include <memory>


namespace memoria {
namespace v1 {

template <
    typename MapName
>
class MultiMapBasicTest: public MultiMapTestBase<MapName> {

    using MyType = MultiMapBasicTest<MapName>;
    using Base   = MultiMapTestBase<MapName>;


    using typename Base::Iterator;
    using typename Base::Key;
    using typename Base::Value;
    using typename Base::Ctr;


    using Base::commit;
    using Base::drop;
    using Base::branch;
    using Base::allocator;
    using Base::snapshot;
    using Base::check;
    using Base::getRandom;

    using Base::checkData;
    using Base::checkRunPositions;
    using Base::out;

    using Base::sampleTreeShape;
    using Base::createRandomShapedMapData;
    using Base::createRandomShapedVectorMap;
    using Base::make_key;
    using Base::make_value;

public:

    MultiMapBasicTest(StringRef name): Base(name)
    {
        MEMORIA_ADD_TEST(runCreateTest);
        MEMORIA_ADD_TEST(runAssignTest);
        MEMORIA_ADD_TEST(runRemoveTest);
    }

    void runCreateTest()
    {
        auto snp = branch();
        auto map = create<MapName>(snp);

        auto shape = sampleTreeShape();

        auto map_data = createRandomShapedMapData(
                shape[0],
                shape[1],
                [this](auto k) {return this->make_key(k, TypeTag<Key>());},
                [this](auto k, auto v) {return this->make_value(this->getRandom(), TypeTag<Value>());}
        );

        mmap::MultimapIOBufferProducer<Key, Value> stream_adaptor(map_data, 65536);

        auto iter = map.begin();
        iter.insert_subseq(stream_adaptor);

        checkData(map, map_data);
        checkRunPositions(map);

        snp.commit();
    }
    
    void runAssignTest()
    {
        auto snp = branch();
        auto map = create<MapName>(snp);

        auto shape = sampleTreeShape();

        auto vector_map = createRandomShapedVectorMap(
                shape[0],
                shape[1],
                [this](auto k) {return this->make_key(k, TypeTag<Key>());},
                [this](auto k, auto v) {return this->make_value(this->getRandom(), TypeTag<Value>());}
        );

        std::vector<Key> keys;

        
        for (const auto& entry: vector_map) {
            keys.push_back(entry.first);
        }
        
        std::random_shuffle(keys.begin(), keys.end());
        
        for (auto& key: keys) 
        {
            const auto& value = vector_map[key];
            
            map.assign(
                key,  
                value.begin(),  
                value.end()
            );
        }

        auto iter = map.begin();
        
        checkData(map, vector_map);
        checkRunPositions(map);

        snp.commit();
    }
    
    
    void runRemoveTest()
    {
        auto snp = branch();
        auto map = create<MapName>(snp);

        auto shape = sampleTreeShape();

        auto vector_map = createRandomShapedVectorMap(
                shape[0],
                shape[1],
                [this](auto k) {return this->make_key(k, TypeTag<Key>());},
                [this](auto k, auto v) {return this->make_value(this->getRandom(), TypeTag<Value>());}
        );

        std::vector<Key> keys;

        for (const auto& entry: vector_map) {
            keys.push_back(entry.first);
        }
        
        std::random_shuffle(keys.begin(), keys.end());
        
        for (auto& key: keys) 
        {
            const auto& value = vector_map[key];
            
            map.assign(
                key,  
                value.begin(),  
                value.end()
            );
        }
        
        size_t step_size = keys.size() / 10;
        if (step_size == 0) step_size = 1;
        
        int32_t cnt{};
        for (auto& key: keys) 
        {
            map.remove(key);
            vector_map.erase(key);
            
            if (cnt % step_size == 0) 
            {
                out() << "Check data: size = " << map.size() << std::endl;
                checkData(map, vector_map);
                checkRunPositions(map);
            }
            
            cnt++;
        }

        checkData(map, vector_map);
        checkRunPositions(map);

        snp.commit();
    }
};

}}