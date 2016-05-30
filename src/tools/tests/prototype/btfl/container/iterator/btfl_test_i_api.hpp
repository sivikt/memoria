
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

#include <memoria/v1/core/types/types.hpp>
#include <memoria/v1/core/types/algo/for_each.hpp>

#include <memoria/v1/prototypes/bt_fl/btfl_names.hpp>
#include <memoria/v1/core/container/iterator.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include "../btfl_test_names.hpp"

#include <iostream>

namespace memoria {
namespace v1 {


MEMORIA_V1_ITERATOR_PART_BEGIN(v1::btfl_test::IterApiName)


    using typename Base::Container;


    using CtrSizeT  = typename Container::Types::CtrSizeT;
    using Key       = typename Container::Types::Key;
    using Value     = typename Container::Types::Value;


    using LeafDispatcher = typename Container::Types::Pages::LeafDispatcher;

    static const Int Streams          = Container::Types::Streams;
    static const Int DataStreams      = Container::Types::DataStreams;

public:

    template <typename IOBuffer>
    auto bulkio_insert(BufferProducer<IOBuffer>& provider, const Int initial_capacity = 20000)
    {
        auto& self = this->self();

        return self.ctr().bulkio_insert(self, provider, initial_capacity);
    }

MEMORIA_V1_ITERATOR_PART_END

#define M_TYPE      MEMORIA_V1_ITERATOR_TYPE(v1::btfl_test::IterApiName)
#define M_PARAMS    MEMORIA_V1_ITERATOR_TEMPLATE_PARAMS

#undef M_TYPE
#undef M_PARAMS

}}
