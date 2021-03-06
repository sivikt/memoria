
// Copyright 2019 Victor Smirnov
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


#include <memoria/core/datatypes/datatypes.hpp>

#include <memoria/profiles/default/default.hpp>
#include <memoria/api/store/memory_store_api.hpp>

#include <memoria/api/set/set_api.hpp>
#include <memoria/core/tools/time.hpp>
#include <memoria/core/tools/random.hpp>
#include <memoria/memoria.hpp>

#include <iostream>
#include <set>
#include <vector>

using namespace memoria;

UUID make_random_uuid() {
    auto hi = getBIRandomG();
    auto lo = getBIRandomG();
    return UUID(
        static_cast<uint64_t>(hi),
        static_cast<uint64_t>(lo)
    );
}

int main()
{
    StaticLibraryCtrs<>::init();

    try {
        using CtrType = Set<UUID>;

        auto alloc = IMemoryStore<>::create().get_or_throw();

        auto snp = alloc->master().get_or_throw()->branch().get_or_throw();

        UUID ctr_id = UUID::parse("30b33f35-96a4-4502-82f4-4bbe50e59c51");

        auto ctr0 = create(snp, CtrType(), ctr_id).get_or_throw();
        ctr0->set_new_block_size(1024).get_or_throw();

        int64_t time = (int64_t)getTimeInMillis();
        std::cout << "Seed: " << time << std::endl;
        SeedBI(time);

        //Seed: 1580702615321
        //SeedBI(1580703467476);

        int64_t t0 = getTimeInMillis();

        std::set<UUID> entries;
        std::vector<UUID> entries_list;

        for (int c = 0; c < 100000000; c++) {
            if (c % 100000 == 0) {
                std::cout << "C=" << c << std::endl;
            }

            UUID key = make_random_uuid();
            entries_list.push_back(key);

            entries.insert(key);

            ctr0->insert(key).get_or_throw();
        }

        std::cout << "Size: " << ctr0->size().get_or_throw() << std::endl;


        int64_t t1 = getTimeInMillis();

        std::cout << "Populated entries in " << (t1 - t0) << " ms" << std::endl;

        int64_t t2 = getTimeInMillis();
        for (auto key: entries_list) {
            bool kk = ctr0->contains(key).get_or_throw();
            if (!kk) {
                std::cout << "Not found: " << key << std::endl;
            }
        }
        int64_t t3 = getTimeInMillis();
        std::cout << "Queried entries in " << (t3 - t2) << " ms" << std::endl;

//        auto scc = ctr0->scanner();
//        auto en_ii = entries.begin();

//        while (!scc.is_end())
//        {
//            for (auto key: scc.keys())
//            {
//                auto en_key = *en_ii;

//                if (key != en_key.view()) {
//                    std::cout << "SCC: not equals: " << en_key << " :: " << key << std::endl;
//                }

//                en_ii++;
//            }

//            scc.next_leaf().get_or_throw();
//        }

        size_t cnt = 0;
        for (auto& key: entries_list)
        {
            if (cnt % 100000 == 0) {
                std::cout << "K=" << cnt << std::endl;
            }

            ctr0->remove(key).get_or_throw();
            cnt++;
        }

        std::cout << "Size: " << ctr0->size().get_or_throw() << std::endl;

        snp->commit().get_or_throw();
        snp->set_as_master().get_or_throw();
        alloc->store("set_insq2.mma1").get_or_throw();
    }
    catch (MemoriaThrowable& th) {
        th.dump(std::cerr);
    }
    catch (ResultException& th) {
        std::cout << "[" << th.what() << "]" << std::endl;
    }

    return 0;
}
