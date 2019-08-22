
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

#include <memoria/v1/api/vector/vector_api.hpp>
#include <memoria/v1/core/container/ctr_impl_btss.hpp>

#include <memory>

namespace memoria {
namespace v1 {

template <typename Value, typename IteratorPtr>
class VectorIteratorImpl: public VectorIterator<Value> {
    using ValueV   = typename DataTypeTraits<Value>::ValueType;
    IteratorPtr iter_;

public:
    VectorIteratorImpl(IteratorPtr iter):
        iter_(iter)
    {}

    virtual ValueV value() const
    {
        return iter_->value();
    }

    virtual bool is_end() const
    {
        return iter_->isEnd();
    }

    virtual void next() {
        iter_->next();
    }
};


template <typename Value, typename Profile>
void ICtrApi<Vector<Value>, Profile>::init_profile_metadata()
{
    SharedCtr<Vector<Value>, ProfileAllocatorType<Profile>, Profile>::init_profile_metadata();
}


    
}}
