
// Copyright 2017 Victor Smirnov
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

#include <memoria/v1/core/types.hpp>
#include <memoria/v1/core/tools/uuid.hpp>

#include <memoria/v1/core/strings/string.hpp>

#include <memoria/v1/profiles/common/common.hpp>


#include <string>

namespace memoria {
namespace v1 {

template <typename Profile>
struct CtrReferenceable {
    virtual bool is_castable_to(uint64_t type_hash) const   = 0;
    virtual U16String describe_type() const                 = 0;
    virtual uint64_t type_hash()                            = 0;

    virtual void set_new_block_size(int32_t block_size)     = 0;
    virtual int32_t get_new_block_size()                    = 0;
    
    virtual const ProfileCtrID<Profile>& name() const = 0;
    
    virtual ~CtrReferenceable() noexcept {}
};


}}
