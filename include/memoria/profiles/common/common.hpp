
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

#include <memoria/core/types.hpp>

namespace memoria {

template <typename Profile> struct ProfileTraits;

template <typename Profile>
using ProfileBlockType = typename ProfileTraits<Profile>::BlockType;

template <typename Profile>
using ProfileAllocatorType = typename ProfileTraits<Profile>::AllocatorType;

template <typename Profile>
using ProfileBlockID = typename ProfileTraits<Profile>::BlockID;

template <typename Profile>
using ProfileCtrID = typename ProfileTraits<Profile>::CtrID;

template <typename Profile>
using ProfileSnapshotID = typename ProfileTraits<Profile>::SnapshotID;

template <typename Profile>
using ProfileCtrSizeT = typename ProfileTraits<Profile>::CtrSizeT;

template <typename Profile>
bool ProfileIsCopyOnWrite = ProfileTraits<Profile>::IsCoW;

template <typename Profile>
using ProfileBlockG = typename ProfileTraits<Profile>::BlockG;

template <typename ID> struct IDTools;

template <typename Profile>
struct ProfileSpecificBlockTools;

}
