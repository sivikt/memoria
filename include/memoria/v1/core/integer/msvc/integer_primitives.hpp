
// Copyright 2018 Victor Smirnov
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

#include <iostream>

namespace memoria {
namespace v1 {

namespace _ {

template <typename V> struct BuiltinAddCH;
template <typename V> struct BuiltinSubCH;

template <>
struct BuiltinAddCH<unsigned long> {
    static unsigned long process(unsigned long x, unsigned long y, unsigned long carry_in, unsigned long& carry_out) {
        return 0;//__builtin_addcl(x, y, carry_in, &carry_out);
    }
};

template <>
struct BuiltinAddCH<unsigned long long> {
    static unsigned long long process(unsigned long long x, unsigned long long y, unsigned long long carry_in, unsigned long long& carry_out) {
        return 0;//__builtin_addcll(x, y, carry_in, &carry_out);
    }
};

template <>
struct BuiltinSubCH<unsigned long> {
    static unsigned long process(unsigned long x, unsigned long y, unsigned long carry_in, unsigned long& carry_out) {
        return 0;//__builtin_subcl(x, y, carry_in, &carry_out);
    }
};

template <>
struct BuiltinSubCH<unsigned long long> {
    static unsigned long long process(unsigned long long x, unsigned long long y, unsigned long long carry_in, unsigned long long& carry_out) {
        return 0;//__builtin_subcll(x, y, carry_in, &carry_out);
    }
};

static inline bool long_add_to(uint64_t* dest, const uint64_t* source, size_t length)
{
    uint64_t carry_in{};
    uint64_t carry_out{};

    for (size_t c = 0; c < length; c++)
    {
        dest[c] = BuiltinAddCH<uint64_t>::process(dest[c], source[c], carry_in, carry_out);
        carry_in = carry_out;
    }

    return carry_out == 0;
}

static inline bool long_add_to(uint64_t* dest, const uint64_t* x, const uint64_t* y, size_t length)
{
    uint64_t carry_in{};
    uint64_t carry_out{};

    for (size_t c = 0; c < length; c++)
    {
        dest[c] = BuiltinAddCH<uint64_t>::process(x[c], y[c], carry_in, carry_out);
        carry_in = carry_out;
    }

    return carry_out == 0;
}


static inline bool long_sub_from(uint64_t* dest, const uint64_t* op, size_t length)
{
    uint64_t carry_in{};
    uint64_t carry_out{};

    for (size_t c = 0; c < length; c++)
    {
        dest[c] = BuiltinSubCH<uint64_t>::process(dest[c], op[c], carry_in, carry_out);
        carry_in = carry_out;
    }

    return carry_out == 0;
}

static inline bool long_sub_from(uint64_t* dest, const uint64_t* x, const uint64_t* y, size_t length)
{
    uint64_t carry_in{};
    uint64_t carry_out{};

    for (size_t c = 0; c < length; c++)
    {
        dest[c] = BuiltinSubCH<uint64_t>::process(x[c], y[c], carry_in, carry_out);
        carry_in = carry_out;
    }

    return carry_out == 0;
}


}

}}
