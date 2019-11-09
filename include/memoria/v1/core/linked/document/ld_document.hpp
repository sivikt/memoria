
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

#include <memoria/v1/core/linked/document/ld_common.hpp>
#include <memoria/v1/core/exceptions/exceptions.hpp>

#include <functional>

namespace memoria {
namespace v1 {

struct SDNParseException: RuntimeException {};

class SDNParserConfiguration {
public:
    SDNParserConfiguration() {}
};

class LDDocument;

class LDDocumentView {

protected:
    using ValueMap      = sdn2_::ValueMap;
    using StringSet     = sdn2_::StringSet;
    using Array         = sdn2_::Array;
    using TypeDeclsMap  = sdn2_::TypeDeclsMap;
    using DocumentState = sdn2_::DocumentState;
    using DocumentPtr   = SDN2Ptr<DocumentState>;

    SDN2ArenaView arena_;

    friend class LDDocumentBuilder;
    friend class LDDMap;
    friend class LDDArray;
    friend class LDTypeName;
    friend class LDDataTypeParam;
    friend class LDDataTypeCtrArg;
    friend class LDTypeDeclaration;
    friend class LDDValue;
    friend class LDString;
    friend class LDIdentifier;
    friend class LDDTypedValue;

    using CharIterator = typename U8StringView::const_iterator;

public:
    LDDocumentView(): arena_() {}
    LDDocumentView(SDN2ArenaView arena): arena_(arena) {}

    LDDValue value() const;

    void set(U8StringView string);
    void set(int64_t value);
    void set(double value);

    LDDMap set_map();
    LDDArray set_array();

    LDDValue set_sdn(U8StringView sdn);

    LDDocumentView* make_mutable() const
    {
        if (arena_.is_mutable()) {
            return const_cast<LDDocumentView*>(this);
        }
        MMA1_THROW(RuntimeException()) << WhatCInfo("Document is not mutable");
    }



    std::ostream& dump(std::ostream& out) const
    {
        LDDumpFormatState state;
        LDDumpState dump_state(*this);
        dump(out, state, dump_state);
        return out;
    }

    std::ostream& dump(std::ostream& out, LDDumpFormatState& state, LDDumpState& dump_state) const;

    LDTypeDeclaration create_named_type(U8StringView name, U8StringView type_decl);

    Optional<LDTypeDeclaration> get_named_type_declaration(U8StringView name) const;

    void remove_named_type_declaration(U8StringView name);

    void for_each_named_type(std::function<void (U8StringView name, LDTypeDeclaration)> fn) const;

    static bool is_identifier(U8StringView string) {
        return is_identifier(string.begin(), string.end());
    }

    static bool is_identifier(CharIterator start, CharIterator end);

    static void assert_identifier(U8StringView name);

protected:
    DocumentPtr doc_ptr() const {
        return DocumentPtr{sizeof(SDN2Header)};
    }

    LDTypeDeclaration parse_raw_type_decl(
            CharIterator start,
            CharIterator end,
            const SDNParserConfiguration& cfg = SDNParserConfiguration{}
    );

    LDDValue parse_raw_value(
            CharIterator start,
            CharIterator end,
            const SDNParserConfiguration& cfg = SDNParserConfiguration{}
    );

    void do_dump_dictionary(std::ostream& out, LDDumpFormatState& state, LDDumpState& dump_state) const;

    bool has_type_dictionary() const {
        return state()->type_directory.get() != 0;
    }

    void set_named_type_declaration(LDIdentifier name, LDTypeDeclaration type_decl);

    LDTypeDeclaration new_type_declaration(U8StringView name);
    LDTypeDeclaration new_type_declaration(LDIdentifier name);
    LDDTypedValue new_typed_value(LDTypeDeclaration typedecl, LDDValue ctr_value);

    LDTypeDeclaration new_detached_type_declaration(LDIdentifier name);

    TypeDeclsMap ensure_type_decls_exist()
    {
        auto* state = this->state_mutable();
        if (!state->type_directory)
        {
            TypeDeclsMap vv = TypeDeclsMap::create(arena_.make_mutable());
            this->state_mutable()->type_directory = vv.ptr();
            return vv;
        }

        return TypeDeclsMap::get(&arena_, state->type_directory);
    }

    LDString new_string(U8StringView view);
    LDIdentifier new_identifier(U8StringView view);

    LDDValue new_integer(int64_t value);
    LDDValue new_double(double value);
    LDDArray new_array(Span<LDDValue> span);
    LDDArray new_array();
    LDDMap new_map();

    void set_value(LDDValue value);

    SDN2Ptr<U8LinkedString> intern(U8StringView view);

    DocumentState* state_mutable() {
        return doc_ptr().get_mutable(&arena_);
    }

    const DocumentState* state() const {
        return doc_ptr().get(&arena_);
    }

    void set_tag(SDN2PtrHolder ptr, LDDValueTag tag)
    {
        SDN2Ptr<LDDValueTag> tag_ptr(ptr - sizeof(LDDValueTag));
        *tag_ptr.get_mutable(&arena_) = tag;
    }
};


class LDDocument: public LDDocumentView {
    SDN2Arena ld_arena_;

    using LDDocumentView::arena_;

    friend class LDDArray;
    friend class LDDMap;
    friend class LDTypeDeclaration;
    friend class LDDTypedValue;

    static constexpr size_t INITIAL_ARENA_SIZE = sizeof(SDN2Header) + sizeof(DocumentState) + 16;

public:
    LDDocument():
        LDDocumentView(),
        ld_arena_(INITIAL_ARENA_SIZE, &arena_)
    {
        allocate<DocumentState>(ld_arena_.view(), DocumentState{0, 0, 0});
    }

    LDDocument(const LDDocument& other):
        LDDocumentView(),
        ld_arena_(&arena_, other.ld_arena_)
    {}

    LDDocument(LDDocument&& other):
        LDDocumentView(),
        ld_arena_(&arena_, std::move(other.ld_arena_))
    {}

    LDDocument& operator=(const LDDocument&) = delete;
    LDDocument& operator=(LDDocument&&);

    void compactify();

    static LDDocument parse(U8StringView view) {
        return parse(view.begin(), view.end());
    }

    static LDDocument parse(
            CharIterator start,
            CharIterator end,
            const SDNParserConfiguration& cfg = SDNParserConfiguration{}
    );

    static LDDocument parse_type_decl(
            U8StringView view,
            const SDNParserConfiguration& cfg = SDNParserConfiguration{}
    ) {
        return parse_type_decl(view.begin(), view.end());
    }

    static LDDocument parse_type_decl(
            CharIterator start,
            CharIterator end,
            const SDNParserConfiguration& cfg = SDNParserConfiguration{}
    );
};


}}
