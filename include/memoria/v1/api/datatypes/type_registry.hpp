
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

#include <memoria/v1/api/datatypes/type_signature.hpp>
#include <memoria/v1/api/datatypes/traits.hpp>
#include <memoria/v1/api/datatypes/datum_base.hpp>

#include <memoria/v1/core/types/list/typelist.hpp>
#include <memoria/v1/core/exceptions/exceptions.hpp>

#include <memoria/v1/core/linked/document/linked_document.hpp>

#include <boost/any.hpp>
#include <boost/context/detail/apply.hpp>

#include <unordered_map>
#include <mutex>
#include <tuple>

namespace memoria {
namespace v1 {



namespace _ {
    template <typename T, typename ParamsList, typename... CtrArgsLists> struct DataTypeCreator;

    template <
        typename T,
        typename Registry,
        typename SerializerFn,
        bool SdnDeserializable = DataTypeTraits<T>::isSdnDeserializable
    >
    struct SDNSerializerFactory {
        static SerializerFn get_deserializer()
        {
            return [](const Registry& registry, const LDDocument& decl)
            {
                return Datum<T>::from_sdn(decl);
            };
        }
    };

    template <typename T, typename Registry, typename SerializerFn>
    struct SDNSerializerFactory<T, Registry, SerializerFn, false> {
        static SerializerFn get_deserializer()
        {
            return SerializerFn();
        }
    };

}




class DataTypeRegistry {
    using CreatorFn   = std::function<boost::any (const DataTypeRegistry&, const LDTypeDeclaration&)>;
    using SdnParserFn = std::function<AnyDatum (const DataTypeRegistry&, const LDDocument&)>;

    std::map<U8String, std::tuple<CreatorFn, SdnParserFn>> creators_;

public:
    friend class DataTypeRegistryStore;

    DataTypeRegistry();

    boost::any create_object(const LDTypeDeclaration& decl) const
    {
        U8String typedecl = decl.to_cxx_typedecl();
        auto ii = creators_.find(typedecl);
        if (ii != creators_.end())
        {
            return std::get<0>(ii->second)(*this, decl);
        }
        else {
            MMA1_THROW(RuntimeException()) << fmt::format_ex(u"Type creator for {} is not registered", typedecl);
        }
    }

    AnyDatum from_sdn_string(U8StringView sdn_string) const;

    static DataTypeRegistry& local();

    void refresh();
};

class DataTypeRegistryStore {

    using CreatorFn   = typename DataTypeRegistry::CreatorFn;
    using SdnParserFn = typename DataTypeRegistry::SdnParserFn;

    std::map<U8String, std::tuple<CreatorFn, SdnParserFn>> creators_;

    mutable std::mutex mutex_;

    using LockT = std::lock_guard<std::mutex>;

public:
    friend class DataTypeRegistry;

    DataTypeRegistryStore() {}


    template <typename T>
    void register_creator(CreatorFn creator, SdnParserFn parser)
    {
        LockT lock(mutex_);

        TypeSignature ts = make_datatype_signature<T>();
        creators_[ts.parse().to_cxx_typedecl()] = std::make_tuple(creator, parser);
    }

    template <typename T>
    void unregister_creator()
    {
        LockT lock(mutex_);

        TypeSignature ts = make_datatype_signature<T>();

        U8String decl = ts.parse().to_cxx_typedecl();

        auto ii = creators_.find(decl);

        if (ii != creators_.end())
        {
            creators_.erase(ii);
        }
    }

    template <typename T, typename... ArgTypesLists>
    void register_creator_fn()
    {
        auto creator_fn = [](const DataTypeRegistry& registry, const LDTypeDeclaration& decl)
        {
            constexpr size_t declared_params_size = ListSize<typename DataTypeTraits<T>::Parameters>;

            size_t actual_parameters_size = decl.params();

            if (declared_params_size == actual_parameters_size)
            {
                return _::DataTypeCreator<
                        T,
                        typename DataTypeTraits<T>::Parameters,
                        ArgTypesLists...
                >::create(registry, decl);
            }
            else {
                MMA1_THROW(RuntimeException())
                        << fmt::format_ex(
                               u"Actual number of parameters {} does not match expected one {} for type {}",
                               actual_parameters_size, declared_params_size,
                               decl.to_standard_string()
                           );
            }
        };

        auto parser_fn = _::SDNSerializerFactory<T, DataTypeRegistry, SdnParserFn>::get_deserializer();

        register_creator<T>(creator_fn, parser_fn);
    }

    static DataTypeRegistryStore& global();

    template <typename T, typename... ArgsTypesLists>
    struct Initializer {
        Initializer() {
            DataTypeRegistryStore::global().register_creator_fn<T, ArgsTypesLists...>();
        }
    };

    template <typename T>
    struct DeInitializer {
        DeInitializer() {
            DataTypeRegistryStore::global().unregister_creator<T>();
        }
    };

private:
    void copy_to(DataTypeRegistry& local)
    {
        LockT lock(mutex_);
        local.creators_.clear();
        local.creators_ = creators_;
    }
};




namespace _ {

    template <typename CxxType>
    struct CtrArgsConverter {
        static bool is_convertible(const LDDValue& value) {
            return false;
        }
    };


    template <>
    struct CtrArgsConverter<bool> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_boolean();
        }

        static bool convert(const LDDValue& value) {
            return value.as_boolean();
        }
    };

    template <>
    struct CtrArgsConverter<int64_t> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_integer();
        }

        static int64_t convert(const LDDValue& value) {
            return value.as_integer();
        }
    };

    template <>
    struct CtrArgsConverter<double> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_double();
        }

        static double convert(const LDDValue& value) {
            return value.as_double();
        }
    };

    template <>
    struct CtrArgsConverter<U8String> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_string();
        }

        static U8String convert(const LDDValue& value) {
            return value.as_string().view();
        }
    };

    template <>
    struct CtrArgsConverter<U8StringView> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_string();
        }

        static U8StringView convert(const LDDValue& value) {
            return value.as_string().view();
        }
    };

    template <>
    struct CtrArgsConverter<LDDValue> {
        static bool is_convertible(const LDDValue& value) {
            return true;
        }

        static LDDValue convert(const LDDValue& value) {
            return value;
        }
    };

    template <>
    struct CtrArgsConverter<LDDArray> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_array();
        }

        static LDDArray convert(const LDDValue& value) {
            return value.as_array();
        }
    };

    template <>
    struct CtrArgsConverter<LDDMap> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_map();
        }

        static LDDMap convert(const LDDValue& value) {
            return value.as_map();
        }
    };

    template <>
    struct CtrArgsConverter<LDTypeDeclaration> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_type_decl();
        }

        static LDTypeDeclaration convert(const LDDValue& value) {
            return value.as_type_decl();
        }
    };

    template <>
    struct CtrArgsConverter<LDDTypedValue> {
        static bool is_convertible(const LDDValue& value) {
            return value.is_typed_value();
        }

        static LDDTypedValue convert(const LDDValue& value) {
            return value.as_typed_value();
        }
    };


    template <size_t Idx, typename Types> struct DataTypeCtrArgsCheckerProc;

    template <size_t Idx, typename ArgType, typename... Types>
    struct DataTypeCtrArgsCheckerProc<Idx, TL<ArgType, Types...>> {

        static bool check(const LDTypeDeclaration& typedecl)
        {
            if (Idx < typedecl.constructor_args())
            {
                if (CtrArgsConverter<std::decay_t<ArgType>>::is_convertible(typedecl))
                {
                    return DataTypeCtrArgsCheckerProc<Idx + 1, TL<Types...>>::check(typedecl);
                }
            }

            return false;
        }
    };

    template <size_t Idx>
    struct DataTypeCtrArgsCheckerProc<Idx, TL<>> {
        static bool check(const LDTypeDeclaration& typedecl) {
            return Idx == typedecl.constructor_args();
        }
    };



    template <size_t Idx, size_t Max>
    struct FillDTTypesList {
        template <typename Tuple>
        static void process(const DataTypeRegistry& registry, const LDTypeDeclaration& typedecl, Tuple& tpl)
        {
            using ParamType = std::tuple_element_t<Idx, Tuple>;

            LDTypeDeclaration param_decl = typedecl.get_type_declration(Idx);

            std::get<Idx>(tpl) = boost::any_cast<std::decay_t<ParamType>>(registry.create_object(param_decl));

            FillDTTypesList<Idx + 1, Max>::process(registry, typedecl, tpl);
        }
    };

    template <size_t Max>
    struct FillDTTypesList<Max, Max> {
        template <typename Tuple>
        static void process(const DataTypeRegistry& registry, const LDTypeDeclaration& typedecl, Tuple& tpl){}
    };


    template <size_t Idx, size_t Max>
    struct FillDTCtrArgsList {
        template <typename Tuple>
        static void process(const LDTypeDeclaration& typedecl, Tuple& tpl)
        {
            using ArgType = std::tuple_element_t<Idx, Tuple>;

            LDDValue arg = typedecl.get_constructor_arg(Idx);

            std::get<Idx>(tpl) = CtrArgsConverter<std::decay_t<ArgType>>::convert(arg);

            FillDTCtrArgsList<Idx + 1, Max>::process(typedecl, tpl);
        }
    };

    template <size_t Max>
    struct FillDTCtrArgsList<Max, Max> {
        template <typename Tuple>
        static void process(const LDTypeDeclaration& typedecl, Tuple& tpl){}
    };



    template<typename Types>
    bool try_to_convert_args(const LDTypeDeclaration& typedecl)
    {
        constexpr int32_t list_size = ListSize<Types>;

        if (typedecl.constructor_args() > 0)
        {
            using Checker = _::DataTypeCtrArgsCheckerProc<0, Types>;
            return Checker::check(typedecl);
        }
        else if (list_size == 0){
            return true;
        }
        else {
            return false;
        }
    }


    template <typename T, typename... ParamsList, typename... ArgsList, typename... ArgsLists>
    struct DataTypeCreator<T, TL<ParamsList...>, TL<ArgsList...>, ArgsLists...>
    {
        static T create(const DataTypeRegistry& registry, const LDTypeDeclaration& typedecl)
        {
            if (_::try_to_convert_args<TL<ArgsList...>>(typedecl))
            {
                std::tuple<ParamsList..., ArgsList...> tpl;

                _::FillDTTypesList<0, sizeof...(ParamsList)>
                        ::process(registry, typedecl, tpl);

                _::FillDTCtrArgsList<sizeof...(ParamsList), sizeof...(ParamsList) + sizeof...(ArgsList)>
                        ::process(typedecl, tpl);

                auto constructorFn = [](auto... args) {
                    return T(args...);
                };

                return boost::context::detail::apply(constructorFn, tpl);
            }
            else {
                return DataTypeCreator<T, TL<ParamsList...>, ArgsLists...>::create(registry, typedecl);
            }
        }
    };

    template <typename T, typename ParamsList>
    struct DataTypeCreator<T, ParamsList>
    {
        static T create(const DataTypeRegistry& registry, const LDTypeDeclaration& typedecl)
        {
            MMA1_THROW(RuntimeException())
                    << fmt::format_ex(
                           u"No constructor is defined for ({}) in type {}",
                           "aaaaaa",
                           TypeNameFactory<T>::name()
                    );
        }
    };

}

}}
