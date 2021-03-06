
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


#include <memoria/profiles/default/default.hpp>
#include <memoria/profiles/memory_cow/memory_cow_profile.hpp>
#include <memoria/memoria.hpp>

namespace memoria {

MMA_DEFINE_EXPLICIT_CU_LINKING(MemoriaStaticInit)

struct StaticInitializer {
    StaticInitializer() {

        InitCoreLDDatatypes();
        InitCoreDatatypes();
        InitSimpleNumericDatatypes();
        InitCtrDatatypes();

#ifdef MEMORIA_BUILD_MEMORY_STORE
        InitDefaultInMemStore();
#endif

#if defined(MEMORIA_BUILD_MEMORY_STORE_COW)
        InitCoWInMemStore();
#endif

#if defined(MEMORIA_BUILD_SWMR_STORE_MAPPED)
        InitSWMRMappedStore();
#endif

#if defined(MEMORIA_BUILD_MEMORY_STORE_COW) || defined(MEMORIA_BUILD_SWMR_STORE_MAPPED)
        StaticLibraryCtrs<MemoryCoWProfile<>>::init();
#endif

        StaticLibraryCtrs<DefaultProfile<>>::init();
    }
};

namespace {

StaticInitializer init0;

}

void InitMemoriaExplicit() {
    StaticInitializer init0;
}

}
