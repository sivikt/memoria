
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

#include <memoria/core/types.hpp>
#include <memoria/core/container/container.hpp>

#include <memoria/api/common/ctr_api.hpp>

#include <memory>

namespace memoria {


template <typename CtrName, typename Allocator, typename Profile>
class SharedCtr: public CtrTF<Profile, CtrName, CtrName>::Type {
    using Base = typename CtrTF<Profile, CtrName, CtrName>::Type;
public:
    SharedCtr(MaybeError& maybe_error, const CtrSharedPtr<Allocator>& allocator, const ProfileCtrID<Profile>& name, CtrName type_decl):
        Base(maybe_error, allocator, name, type_decl)
    {}

    SharedCtr(MaybeError& maybe_error, const CtrSharedPtr<Allocator>& allocator, const typename Allocator::BlockG& root_block):
        Base(maybe_error, allocator, root_block)
    {}    
};


template <typename CtrName, typename Profile>
class SharedIter: public Iter<typename CtrTF<Profile, CtrName, CtrName>::Types::IterTypes> {
    
    using MyType1 = SharedIter<CtrName,Profile>;
    
    using CtrT = typename CtrTF<Profile, CtrName, CtrName>::Type;
    using IterT = Iter<typename CtrTF<Profile, CtrName, CtrName>::Types::IterTypes>;

    using CtrPtr = CtrSharedPtr<CtrT>;
    using ConstCtrPtr = CtrSharedPtr<const CtrT>;
    
    using Base = Iter<typename CtrTF<Profile, CtrName, CtrName>::Types::IterTypes>;
public:
    SharedIter(): Base(){}

    SharedIter(CtrPtr ptr): Base(std::move(ptr))
    {
        Base::iter_local_pos() = 0;
    }

    SharedIter(ConstCtrPtr ptr): Base(std::move(ConstPointerCast<CtrT>(ptr)))
    {
        Base::iter_local_pos() = 0;
    }
    
    SharedIter(const MyType1& other): Base(other) {}
    SharedIter(const IterT& other): Base(other) {}
};


}
