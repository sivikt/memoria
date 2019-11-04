
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
#include <memoria/v1/core/strings/u8_string.hpp>

#include <memoria/v1/core/linked/common/arena.hpp>
#include <memoria/v1/core/linked/common/linked_string.hpp>
#include <memoria/v1/core/linked/common/linked_dyn_vector.hpp>
#include <memoria/v1/core/linked/common/linked_vector.hpp>
#include <memoria/v1/core/linked/common/linked_map.hpp>
#include <memoria/v1/core/linked/common/linked_set.hpp>

#include <unordered_map>

namespace memoria {
namespace v1 {

struct SDN2Header {
    uint32_t version;
};

using SDN2Arena     = LinkedArena<SDN2Header, uint32_t>;
using SDN2ArenaBase = typename SDN2Arena::ArenaBase;
using SDN2PtrHolder = SDN2ArenaBase::PtrHolderT;
using LDDValueTag   = uint16_t;

template <typename T>
using SDN2Ptr = typename SDN2Arena::template PtrT<T>;




namespace sdn2_ {

    using PtrHolder = SDN2ArenaBase::PtrHolderT;

    using ValueMap = LinkedMap<
        SDN2Ptr<U8LinkedString>,
        PtrHolder,
        SDN2ArenaBase,
        LinkedPtrHashFn,
        LinkedStringPtrEqualToFn
    >;

    using StringSet = LinkedSet<
        SDN2Ptr<U8LinkedString>,
        SDN2ArenaBase,
        LinkedPtrHashFn,
        LinkedStringPtrEqualToFn
    >;

    using MapState = typename ValueMap::State;

    using Array = LinkedDynVector<PtrHolder, SDN2ArenaBase>;
    using ArrayState = typename Array::State;

    struct TypeDeclState;

    using TypeDeclPtr = SDN2Ptr<TypeDeclState>;

    struct TypeDeclState {
        SDN2Ptr<U8LinkedString> name;
        SDN2Ptr<LinkedVector<TypeDeclPtr>> type_params;
        SDN2Ptr<LinkedVector<PtrHolder>> ctr_args;
    };

    struct TypedValueState {
        SDN2Ptr<TypeDeclState> type_decl;
        PtrHolder value_ptr;
    };

    using TypeDeclsMap = LinkedMap<
        SDN2Ptr<U8LinkedString>,
        PtrHolder,
        SDN2ArenaBase,
        LinkedPtrHashFn,
        LinkedStringPtrEqualToFn
    >;

    struct DocumentState {
        PtrHolder value;
        SDN2Ptr<TypeDeclsMap::State> type_directory;
        SDN2Ptr<sdn2_::StringSet::State> strings;
    };


}



class LDDArray;

class LDDValue;
class LDDMap;
class LDTypeDeclaration;
class LDTypeName;
class LDDataTypeParam;
class LDDataTypeCtrArg;
class LDDocument;
class LDString;
class LDIdentifier;
class LDDTypedValue;

class LDDocumentBuilder;


template <typename T> struct LDDValueTraits;

template <>
struct LDDValueTraits<LDString> {
    static constexpr LDDValueTag ValueTag = 1;
};

template <>
struct LDDValueTraits<int64_t> {
    static constexpr LDDValueTag ValueTag = 2;
};

template <>
struct LDDValueTraits<double> {
    static constexpr LDDValueTag ValueTag = 3;
};

template <>
struct LDDValueTraits<LDDMap> {
    static constexpr LDDValueTag ValueTag = 5;
};

template <>
struct LDDValueTraits<LDDArray> {
    static constexpr LDDValueTag ValueTag = 6;
};

template <>
struct LDDValueTraits<LDTypeDeclaration> {
    static constexpr LDDValueTag ValueTag = 7;
};

template <>
struct LDDValueTraits<LDIdentifier> {
    static constexpr LDDValueTag ValueTag = LDDValueTraits<LDString>::ValueTag;
};

template <>
struct LDDValueTraits<LDDTypedValue> {
    static constexpr LDDValueTag ValueTag = 9;
};



class LDDumpFormatState {
    const char* space_;

    const char* nl_start_;
    const char* nl_middle_;
    const char* nl_end_;

    size_t indent_size_;
    size_t current_indent_;

public:
    LDDumpFormatState(
            const char* space,
            const char* nl_start,
            const char* nl_middle,
            const char* nl_end,
            size_t indent_size
    ):
        space_(space),
        nl_start_(nl_start),
        nl_middle_(nl_middle),
        nl_end_(nl_end),
        indent_size_(indent_size),
        current_indent_(0)
    {}

    LDDumpFormatState(): LDDumpFormatState(" ", "\n", "\n", "\n", 2) {}

    static LDDumpFormatState no_indent() {
        return LDDumpFormatState("", "", "", "", 0);
    }

    LDDumpFormatState simple() {
        return LDDumpFormatState("", "", " ", "", 0);
    }

    const char* space() const {return space_;}

    const char* nl_start() const {return nl_start_;}
    const char* nl_middle() const {return nl_middle_;}
    const char* nl_end() const {return nl_end_;}

    size_t indent_size() const {return indent_size_;}
    size_t current_indent() const {return current_indent_;}

    void push() {
        current_indent_ += indent_size_;
    }

    void pop() {
        current_indent_ -= indent_size_;
    }

    void make_indent(std::ostream& out) const {
        for (size_t c = 0; c < current_indent_; c++) {
            out << space_;
        }
    }
};

class LDDumpState {
    std::unordered_map<SDN2PtrHolder, U8StringView> type_mapping_;
public:
    LDDumpState(const LDDocument& doc);

    Optional<U8StringView> resolve_type_id(SDN2PtrHolder ptr) const
    {
        auto ii = type_mapping_.find(ptr);
        if (ii != type_mapping_.end()) {
            return ii->second;
        }

        return Optional<U8StringView>{};
    }
};

}}
