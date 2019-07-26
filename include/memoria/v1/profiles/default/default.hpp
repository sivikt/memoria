
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


#pragma once

#include <memoria/v1/profiles/common/common.hpp>
#include <memoria/v1/profiles/common/block.hpp>
#include <memoria/v1/core/container/allocator.hpp>

#include <memoria/v1/core/tools/uuid.hpp>

namespace memoria {
namespace v1 {

template <typename Profile>
struct ProfileTraits {
    using BlockID       = UUID;
    using SnapshotID    = UUID;
    using CtrID         = UUID;
    using CtrSizeT      = int64_t;

    using Page = AbstractPage <BlockID, 32>;
    using BlockType = Page;

    using AllocatorType = IAllocator<Profile>;
};

template <>
struct IDTools<UUID> {
    static UUID make_random() {
        return UUID::make_random();
    }
};

}}