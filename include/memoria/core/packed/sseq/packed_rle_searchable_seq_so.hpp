
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

#include <memoria/profiles/common/block_operations.hpp>

#include <memoria/core/iovector/io_substream_base.hpp>

namespace memoria {

template <typename ExtData, typename PkdStruct>
class PackedRLESeqSO {
    const ExtData* ext_data_;
    PkdStruct* data_;

    using MyType = PackedRLESeqSO;

public:
    using PkdStructT = PkdStruct;

    PackedRLESeqSO(): ext_data_(), data_() {}
    PackedRLESeqSO(const ExtData* ext_data, PkdStruct* data):
        ext_data_(ext_data), data_(data)
    {}

    void setup() {
        ext_data_ = nullptr;
        data_ = nullptr;
    }

    void setup(const ExtData* ext_data, PkdStruct* data) {
        ext_data_ = ext_data;
        data_ = data;
    }

    void setup(const ExtData* ext_data) {
        ext_data_ = ext_data;
    }

    void setup(PkdStruct* data) {
        data_ = data;
    }


    operator bool() const {
        return data_ != nullptr;
    }

    const ExtData* ext_data() const {return ext_data_;}
    const PkdStruct* data() const {return data_;}
    PkdStruct* data() {return data_;}

    VoidResult splitTo(MyType& other, int32_t idx) noexcept
    {
        return data_->splitTo(other.data(), idx);
    }

    VoidResult mergeWith(MyType& other) noexcept {
        return data_->mergeWith(other.data());
    }

    VoidResult removeSpace(int32_t room_start, int32_t room_end) noexcept {
        return data_->removeSpace(room_start, room_end);
    }

    int32_t size() const {
        return data_->size();
    }

    template <typename... Args>
    auto selectGEFW(Args&&... args) const {
        return data_->selectGEFW(std::forward<Args>(args)...);
    }

    template <typename... Args>
    auto selectFW(Args&&... args) const {
        return data_->selectFW(std::forward<Args>(args)...);
    }

    VoidResult generateDataEvents(IBlockDataEventHandler* handler) const noexcept {
        return data_->generateDataEvents(handler);
    }

    void check() const {
        return data_->check();
    }

    auto sum(int32_t symbol) const noexcept {
        return data_->rank(symbol);
    }

    void configure_io_substream(io::IOSubstream& substream) const {
        return data_->configure_io_substream(substream);
    }

    Int32Result insert_io_substream(int32_t at, const io::IOSubstream& substream, int32_t start, int32_t size) noexcept
    {
        MEMORIA_TRY_VOID(data_->insert_io_substream(at, substream, start, size));
        return Int32Result::of(at + size);
    }

    template <typename AccessorFn>
    VoidResult insert_entries(psize_t row_at, psize_t size, AccessorFn&& elements) noexcept
    {
        MEMORIA_TRY_VOID(data_->insertSpace(row_at, size));

        for (psize_t c = 0; c < size; c++)
        {
            int32_t symbol = elements(c);
            data_->insert(row_at, symbol, 1);
        }

        return VoidResult::of();
    }

    template <typename AccessorFn>
    VoidResult update_entries(psize_t row_at, psize_t size, AccessorFn&& elements) noexcept
    {
        MEMORIA_TRY_VOID(data_->removeSpace(row_at, row_at + size));
        return insert_entries(row_at, size, std::forward<AccessorFn>(elements));
    }

    template <typename AccessorFn>
    VoidResult remove_entries(psize_t row_at, psize_t size) noexcept
    {
        return data_->removeSpace(row_at, row_at + size);
    }

private:
};



}
