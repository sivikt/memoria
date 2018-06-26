
// Copyright 2015 Victor Smirnov
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

#include <memoria/v1/api/common/ctr_api.hpp>

#include <memoria/v1/core/bignum/int64_codec.hpp>
#include <memoria/v1/core/types.hpp>

namespace memoria {
namespace v1 {

template <typename IOBuffer>
struct IBTFLPopulateWalker: Referenceable {
    virtual int32_t populate(IOBuffer* buffer) = 0;
};

namespace _ {

template <typename CtrT, typename WalkerT, typename IOBuffer>
class PopulateWalkerHanlder: public IBTFLPopulateWalker<IOBuffer> {
    CtrT* self_;
    WalkerT walker_;

public:
    PopulateWalkerHanlder(CtrT* self, WalkerT&& walker):
        self_(self), walker_(std::move(walker))
    {}

    virtual int32_t populate(IOBuffer* buffer) {
        return self_->bulkio_populate(*walker_.get(), buffer);
    }
};

}

template <typename IOBuffer, typename CtrT, typename WalkerT>
auto make_btfl_populate_walker_handler(CtrT& ctr, WalkerT&& walker)
{
    return std::unique_ptr<IBTFLPopulateWalker<IOBuffer>>(new _::PopulateWalkerHanlder<CtrT, WalkerT, IOBuffer>{
        &ctr, std::move(walker)
    });
}



}}
