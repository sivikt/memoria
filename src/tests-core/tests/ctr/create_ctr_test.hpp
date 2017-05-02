
// Copyright 2012 Victor Smirnov
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

#include <memoria/v1/tools/profile_tests.hpp>
#include <memoria/v1/tools/tools.hpp>

#include <vector>

#include <memoria/v1/tools/tests_inc.hpp>



namespace memoria {
namespace v1 {



class CreateCtrTest: public SPTestTask {

    typedef CreateCtrTest                                                       MyType;

    typedef KVPair<int64_t, int64_t>                                              Pair;
    typedef vector<Pair>                                                        PairVector;
    typedef DCtrTF<WT>::Type                                                    WTCtr;
    typedef DCtrTF<Map<int64_t, int64_t>>::Type                                   MapCtr;

    PairVector pairs_;

    int32_t map_size_           = 10000;
    int32_t wt_size_            = 500;

    int32_t iteration_          = 0;

    int64_t map_name_;
    int64_t wt_name_;

public:

    CreateCtrTest(): SPTestTask("Create")
    {
        MapCtr::initMetadata();


        MEMORIA_ADD_TEST_PARAM(map_size_)->setDescription("Size of the Map container");
        MEMORIA_ADD_TEST_PARAM(wt_size_)->setDescription("Size of the WaveletTree container");


        MEMORIA_ADD_TEST_PARAM(map_name_)->state();
        MEMORIA_ADD_TEST_PARAM(wt_name_)->state();
        MEMORIA_ADD_TEST_PARAM(iteration_)->state();


        MEMORIA_ADD_TEST(runCreateCtrTest);
        MEMORIA_ADD_TEST(runStoreTest);
    }

    virtual ~CreateCtrTest() noexcept {}


    void assertEmpty(const char* src, Allocator& allocator)
    {
        AssertEQ(src, allocator.size(), 0);
    }

    void assertSize(const char* src, Allocator& allocator, int32_t size)
    {
        AssertEQ(src, allocator.size(), size);
    }

    void runCreateCtrTest()
    {
        DefaultLogHandlerImpl logHandler(out());

        Allocator allocator;
        allocator.getLogger()->setHandler(&logHandler);
        allocator.commit();

        assertEmpty(MA_SRC, allocator);

        AssertThrows<Exception>(MA_SRC, []{
            MapCtr map(nullptr);
        });

        AssertThrows<Exception>(MA_SRC, [&]{
            MapCtr map(&allocator, 0);
        });

        assertEmpty(MA_SRC, allocator);

        AssertThrows<NoCtrException>(MA_SRC, [&]{
            MapCtr map(&allocator, CTR_FIND, 12345);
        });

        assertEmpty(MA_SRC, allocator);

        // Ensure subsequent CTR_FIND with the same name
        // doesn't affect allocator
        AssertThrows<NoCtrException>(MA_SRC, [&]{
            MapCtr map(&allocator, CTR_FIND, 12345);
        });

        assertEmpty(MA_SRC, allocator);

        AssertDoesntThrow(MA_SRC, [&]{
            MapCtr map(&allocator, CTR_CREATE | CTR_FIND, 12345);
        });

        assertSize(MA_SRC, allocator, 1);

        AssertThrows<CtrTypeException>(MA_SRC, [&]{
            WTCtr map(&allocator, CTR_FIND, 12345);
        });

        assertSize(MA_SRC, allocator, 1);

        AssertThrows<NoCtrException>(MA_SRC, [&]{
            WTCtr map(&allocator, CTR_FIND, 12346);
        });

        assertSize(MA_SRC, allocator, 1);

        AssertDoesntThrow(MA_SRC, [&]{
            WTCtr map(&allocator, CTR_FIND | CTR_CREATE, 12346);
        });

        assertSize(MA_SRC, allocator, 2);

        AssertDoesntThrow(MA_SRC, [&]{
            WTCtr map(&allocator, CTR_FIND | CTR_CREATE, 12346);
        });

        assertSize(MA_SRC, allocator, 2);

        AssertDoesntThrow(MA_SRC, [&]{
            WTCtr map(&allocator, CTR_FIND, 12346);
        });

        assertSize(MA_SRC, allocator, 2);

        int64_t name;

        {   // Container object lifecycle scope.

            MapCtr map(&allocator);
            name = map.name();

            assertSize(MA_SRC, allocator, 3);

            for (int32_t c = 0; c < 1000; c++)
            {
                map[c] = c + 1;
            }

            // Container's data still exists in allocator
        }   // after control leaves the cope

        AssertEQ(MA_SRC, name, INITAL_CTR_NAME_COUNTER + 1);

        assertSize(MA_SRC, allocator, 3);

        MapCtr map(&allocator, CTR_FIND, name);

        for (auto pair: map)
        {
            AssertEQ(MA_SRC, pair.first, pair.second - 1);
        }

        //Container removal is not fully implemented yet
        //map.drop();

        assertSize(MA_SRC, allocator, 3);

        allocator.commit();

        assertSize(MA_SRC, allocator, 3);


        int64_t name1 = allocator.createCtrName();

        allocator.rollback();

        int64_t name2 = allocator.createCtrName();

        AssertEQ(MA_SRC, name1, name2);
    }


    void runStoreTest()
    {
        DefaultLogHandlerImpl logHandler(out());

        Allocator allocator;
        allocator.getLogger()->setHandler(&logHandler);

        MapCtr map(&allocator);

        map_name_ = map.name();

        int64_t t00 = getTimeInMillis();

        for (int32_t c = 0; c < map_size_; c++)
        {
            map[getRandom()] = getRandom();
        }

        WTCtr wt_ctr(&allocator);
        wt_ctr.prepare();

        wt_name_ = wt_ctr.name();

        for (int32_t c = 0; c < wt_size_; c++)
        {
            wt_ctr.insert(c, getRandom());
        }

        allocator.commit();

        forceCheck(allocator, MA_SRC);

        int64_t t0 = getTimeInMillis();

        String name = this->getResourcePath("alloc1.dump");

        StoreAllocator(allocator, name);

        int64_t t1 = getTimeInMillis();

        Allocator new_alloc;

        LoadAllocator(new_alloc, name);

        int64_t t2 = getTimeInMillis();

        out()<<"Store Time: "<<FormatTime(t1 - t0)<<endl;
        out()<<"Load Time:  "<<FormatTime(t2 - t1)<<endl;

        forceCheck(new_alloc, MA_SRC);

        MapCtr new_map(&new_alloc, CTR_FIND, map.name());

        AssertEQ(MA_SRC, map.size(), new_map.size());

        auto new_iter = new_map.Begin();

        for (auto iter = map.Begin(); !iter.isEnd(); iter++, new_iter++)
        {
            AssertEQ(MA_SRC, iter.key(), new_iter.key());
            AssertEQ(MA_SRC, iter.value(), new_iter.value());
        }

        int64_t t22 = getTimeInMillis();

        WTCtr new_wt(&new_alloc, CTR_FIND, wt_ctr.name());

        AssertEQ(MA_SRC, wt_ctr.size(), new_wt.size());

        for (int32_t c = 0; c < wt_ctr.size(); c++)
        {
            auto sym1 = wt_ctr.value(c);
            auto sym2 = new_wt.value(c);

            AssertEQ(MA_SRC, sym1, sym2);
        }

        int64_t t33 = getTimeInMillis();

        out()<<"Create Time: "<<FormatTime(t0 - t00)<<endl;
        out()<<"check Time:  "<<FormatTime(t22 - t2)<<endl;
        out()<<"check Time:  "<<FormatTime(t33 - t22)<<endl;
    }


    template <typename T>
    void compareBuffers(const vector<T>& src, const vector<T>& tgt, const char* source)
    {
        AssertEQ(source, src.size(), tgt.size(), SBuf()<<"buffer sizes are not equal");

        for (size_t c = 0; c < src.size(); c++)
        {
            auto v1 = src[c];
            auto v2 = tgt[c];

            AssertEQ(source, v1, v2, [=](){return SBuf()<<"c="<<c;});
        }
    }

};


}}