
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


#include <memoria/v1/core/types.hpp>

#include <memoria/v1/core/strings/string_codec.hpp>

#include <memoria/v1/core/types/type2type.hpp>
#include <memoria/v1/core/tools/bitmap.hpp>
#include <memoria/v1/core/mapped/mapped_hash.hpp>
#include <memoria/v1/core/mapped/arena.hpp>


#include <boost/utility/string_view.hpp>

namespace memoria {
namespace v1 {

template <typename CharT = char> class MappedString;

template <>
class alignas(1) MappedString<char> {

    uint8_t buffer_[1];

public:
    static constexpr bool UseObjectSize = true;

    using CharT = char;
    using ViewType = boost::basic_string_view<CharT>;

    MappedString(ViewType view) noexcept
    {
        ValueCodec<uint64_t> codec;
        size_t len = codec.encode(buffer(), view.length(), 0);
        CharT* data = T2T<CharT*>(buffer() + len);
        MemCpyBuffer(view.data(), data, view.length());
    }

    MappedString(const MappedString& other) noexcept
    {
        ValueCodec<uint64_t> codec;
        size_t len = codec.encode(buffer(), other.size(), 0);
        CharT* data = T2T<CharT*>(buffer() + len);
        MemCpyBuffer(other.data(), data, other.size());
    }

    MappedString(MappedString&&) = delete;


    size_t size() const noexcept
    {
        ValueCodec<uint64_t> size_codec;
        size_t value{};
        size_codec.decode(buffer(), value, 0);
        return value;
    }

    size_t object_size() const noexcept {
        return size_length() + size();
    }

    static size_t object_size(ViewType view)
    {
        size_t str_len = view.length();
        ValueCodec<uint64_t> size_codec;

        size_t size_len = size_codec.length(str_len);

        return size_len + str_len;
    }

    CharT* data() noexcept
    {
        return T2T<CharT*>(buffer() + size_length());
    }

    ViewType view() const noexcept
    {
        return ViewType(data(), size());
    }

    const CharT* data() const noexcept {
        return T2T<const CharT*>(buffer() + size_length());
    }

    bool operator==(ViewType view) const {
        return this->view() == view;
    }



private:
    size_t size_length() const noexcept {
        ValueCodec<uint64_t> size_codec;
        return size_codec.length(buffer(), 0);
    }

    uint8_t* buffer() noexcept {
        return buffer_;
    }

    const uint8_t* buffer() const noexcept {
        return buffer_;
    }
};


template <size_t Size, typename Variant, typename T>
void append(FNVHasher<Size, Variant>& hasher, const MappedString<T>& str)
{
    append(hasher, str.view());
}


template <typename T, typename Arena>
class MappedStringPtrEqualToFn {
    Arena* arena_;
public:
    MappedStringPtrEqualToFn(Arena* arena):
        arena_(arena)
    {}

    template <typename CharT>
    bool operator()(const boost::basic_string_view<CharT>& key, const T& ptr) {
        return (*ptr.get(arena_)) == key;
    }


    bool operator()(const U8String& key, const T& ptr) {
        return (*ptr.get(arena_)).view() == key.view();
    }

    bool operator()(const T key, const T& ptr)
    {
        auto key_view = (*ptr.get(arena_)).view();
        auto ptr_view = (*key.get(arena_)).view();

        return key_view == ptr_view;
    }
};





}}
