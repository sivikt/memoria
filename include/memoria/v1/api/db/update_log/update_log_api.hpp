
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

#include <memoria/v1/api/common/ctr_api_btfl.hpp>
#include <memoria/v1/api/common/ctr_input_btss.hpp>

#include <memoria/v1/api/db/update_log/update_log_input.hpp>
#include <memoria/v1/api/db/update_log/update_log_output.hpp>

#include <memoria/v1/core/tools/uuid.hpp>

#include <memoria/v1/core/tools/object_pool.hpp>



#include <memory>
#include <tuple>

namespace memoria {
namespace v1 {

template <typename Profile>
class CtrApi<UpdateLog, Profile>: public CtrApiBTFLBase<UpdateLog, Profile> {
    
    using Base = CtrApiBTFLBase<UpdateLog, Profile>;
    using typename Base::AllocatorT;
    using typename Base::CtrT;
    using typename Base::CtrPtr;

    using typename Base::Iterator;
    
public:
    using typename Base::CtrSizeT;
    
    static constexpr int32_t DataStreams = 3;
    using CtrSizesT = CtrSizes<Profile, DataStreams + 1>;
    
    using CommandsDataIteratorT = update_log::CommandsDataIterator<Iterator, CtrSizeT>;
    using SnapshotIDIteratorT   = update_log::SnapshotIDIterator<Iterator, CtrSizeT>;

    MMA1_DECLARE_CTRAPI_BASIC_METHODS()

    int64_t size() const;

    void create_snapshot(const UUID& snapshot_id);
    void append_commands(const UUID& ctr_name, bt::BufferProducer<CtrIOBuffer>& data_producer);

    template <typename InputIterator, typename EndIterator>
    void append_commands(const UUID& ctr_name, InputIterator start, EndIterator end)
    {
        InputIteratorProvider<uint8_t, InputIterator, EndIterator, CtrIOBuffer> provider(start, end);
        return append_commands(ctr_name, provider);
    }

    CommandsDataIteratorT read_commads(const UUID& ctr_name, CtrSizeT start = 0);

    bool remove_commands(const UUID& ctr_name, CtrSizeT start, CtrSizeT length);
    bool remove_commands(const UUID& ctr_name);

    SnapshotIDIteratorT find_snapshot(const UUID& snapshot_id);

    SnapshotIDIteratorT latest_snapshot();
};


template <typename Profile>
class IterApi<UpdateLog, Profile>: public IterApiBTFLBase<UpdateLog, Profile> {
    
    using Base = IterApiBTFLBase<UpdateLog, Profile>;
    
    using typename Base::IterT;
    using typename Base::IterPtr;
    
public:
    using typename Base::CtrSizeT;
    
    static constexpr int32_t DataStreams = CtrApi<UpdateLog, Profile>::DataStreams;
    using CtrSizesT = typename CtrApi<UpdateLog, Profile>::CtrSizesT;
    
    MMA1_DECLARE_ITERAPI_BASIC_METHODS()
};


    
}}