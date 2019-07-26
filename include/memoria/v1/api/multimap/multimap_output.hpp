
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

#include <memoria/v1/api/common/ctr_output_btfl_entries.hpp>
#include <memoria/v1/api/common/ctr_output_btfl_run.hpp>

#include <memoria/v1/core/exceptions/exceptions.hpp>
#include <memoria/v1/core/tools/static_array.hpp>
#include <memoria/v1/core/iovector/io_symbol_sequence.hpp>

#include <absl/types/span.h>

#include <memory>
#include <tuple>
#include <exception>
#include <functional>

namespace memoria {
namespace v1 {

template <typename Key, typename Value>
class IEntriesIterator {
protected:
    core::StaticVector<uint64_t, 2> offsets_;
    DefaultBTFLSequenceParser<2> parser_;

public:
    struct Entry {
        const Key* key;
        absl::Span<const Value> values;
    };

protected:
    io::DefaultIOBuffer<Entry> body_{128};

    const Value* prefix_;
    size_t prefix_size_;

    bool has_suffix_;
    Key suffix_key_;
    const Key* suffix_key_ptr_{};

    const Value* suffix_;
    size_t suffix_size_;

    bool use_buffered_{};

    bool buffer_is_ready_{false};
    io::DefaultIOBuffer<Value> suffix_buffer_{128};

    uint64_t iteration_num_{};
public:
    IEntriesIterator(uint32_t start):
        parser_(start)
    {}

    virtual ~IEntriesIterator() noexcept {}

    size_t prefix_size() const {
        return prefix_size_;
    }

    size_t suffix_size() const {
        return suffix_size_;
    }

    Span<const Value> prefix() const {return Span<const Value>{prefix_, prefix_size_};}
    Span<const Value> suffix() const {return Span<const Value>{suffix_, suffix_size_};}

    const Key& suffix_key() const {return suffix_key_;}

    bool has_prefix() const {return prefix_ != nullptr;}
    bool has_suffix() const {return has_suffix_;}
    bool is_first_iteration() const {return iteration_num_ == 1;}

    Span<const Entry> entries() const {
        return body_.span();
    }

    const Entry& entry(size_t idx) const {
        return body_.span()[idx];
    }

    bool has_entries() const {
        return body_.size() > 0;
    }

    Span<const Value> buffer() const {
        return suffix_buffer_.span();
    }

    bool is_buffer_ready() const {
        return buffer_is_ready_;
    }

    bool is_buffered() const {return use_buffered_;}
    virtual void set_buffered() = 0;

    void next_buffer()
    {
        do {
            next();
        }
        while (!is_buffer_ready());
    }

    virtual bool is_end() const     = 0;
    virtual void next()             = 0;
    virtual void dump_iterator() const = 0;


    void for_each_buffered(std::function<void (Key, Span<const Value>)> fn)
    {
        this->set_buffered();

        while (!this->is_end())
        {
            if ((!this->is_first_iteration()) && this->is_buffer_ready())
            {
                fn(this->suffix_key(), this->buffer());
            }

            if (this->has_entries())
            {
                for (auto& entry: this->entries())
                {
                    fn(*entry.key, entry.values);
                }
            }

            this->next();
        }
    }

    void for_each_buffered(int64_t size, std::function<void (Key, Span<const Value>)> fn)
    {
        this->set_buffered();

        int64_t cnt{};

        while (!this->is_end())
        {
            if (cnt == size) {
                return;
            }

            if ((!this->is_first_iteration()) && this->is_buffer_ready())
            {
                fn(this->suffix_key(), this->buffer());
                cnt++;
            }

            if (cnt == size) {
                return;
            }

            if (this->has_entries())
            {
                for (auto& entry: this->entries())
                {
                    if (cnt == size) {
                        return;
                    }

                    fn(*entry.key, entry.values);
                    cnt++;
                }
            }

            this->next();
        }
    }
};


template <typename Value>
class IValuesIterator {
protected:
    const Value* values_{};
    size_t size_{};

    bool use_buffered_{};

    bool buffer_is_ready_{false};

public:
    IValuesIterator()
    {}

    virtual ~IValuesIterator() noexcept {}

    Span<const Value> buffer() const {
        return Span<const Value>{values_, size_};
    }

    bool is_buffer_ready() const {
        return buffer_is_ready_;
    }

    void read_buffer()
    {
        set_buffered();
        while (!is_buffer_ready()) {
            next();
        }
    }

    bool is_buffered() const {return use_buffered_;}
    virtual void set_buffered() = 0;

    virtual bool is_end() const     = 0;
    virtual void next()             = 0;
    virtual void dump_iterator() const = 0;
};


template <typename Key, typename Value>
class IKeysIterator {
protected:
    const Key* keys_{};
    size_t size_{};

    bool use_buffered_{};
    bool buffer_is_ready_{false};

public:
    IKeysIterator() {}

    virtual ~IKeysIterator() noexcept {}

    Span<const Key> buffer() const {
        return Span<const Key>{keys_, size_};
    }

    virtual CtrSharedPtr<IValuesIterator<Value>> values(size_t key_idx) = 0;

    virtual bool is_end() const         = 0;
    virtual void next()                 = 0;
    virtual void dump_iterator() const  = 0;
};

}}