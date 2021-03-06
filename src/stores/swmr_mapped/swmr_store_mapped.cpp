
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

#include <memoria/profiles/memory_cow/memory_cow_profile.hpp>
#include <memoria/store/swmr/mapped/swmr_mapped_store.hpp>

namespace memoria {

using Profile = MemoryCoWProfile<>;
template struct ISWMRStore<Profile>;
template class MappedSWMRStore<Profile>;
template class MappedSWMRStoreWritableCommit<Profile>;
template class MappedSWMRStoreReadOnlyCommit<Profile>;



#if !defined(MEMORIA_BUILD_MEMORY_STORE_COW)
std::ostream& operator<<(std::ostream& out, const MemCoWBlockID<uint64_t>& block_id) noexcept {
    out << block_id.value();
    return out;
}
#endif

namespace {

template <typename PP>
struct Initializer {
    Initializer() {
        MappedSWMRStore<Profile>::init_profile_metadata();
    }
};

}

void InitSWMRMappedStore() {
    Initializer<Profile> init0;
}



Result<SharedPtr<ISWMRStore<MemoryCoWProfile<>>>> open_mapped_swmr_store(U8StringView path)
{
    using ResultT = Result<SharedPtr<ISWMRStore<MemoryCoWProfile<>>>>;

    MaybeError maybe_error;
    auto ptr = MakeShared<MappedSWMRStore<MemoryCoWProfile<>>>(maybe_error, path);

    if (maybe_error) {
        return std::move(maybe_error.get());
    }

    return ResultT::of(ptr);
}

Result<SharedPtr<ISWMRStore<MemoryCoWProfile<>>>> create_mapped_swmr_store(U8StringView path, uint64_t store_size_mb)
{
    using ResultT = Result<SharedPtr<ISWMRStore<MemoryCoWProfile<>>>>;

    MaybeError maybe_error;
    auto ptr = MakeShared<MappedSWMRStore<MemoryCoWProfile<>>>(maybe_error, path, store_size_mb);

    if (maybe_error) {
        return std::move(maybe_error.get());
    }

    return ResultT::of(ptr);
}

}
