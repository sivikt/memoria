
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
#include <memoria/v1/yaml-cpp/yaml.h>
#include <memoria/v1/filesystem/path.hpp>

#include <memoria/v1/api/allocator/allocator_inmem_api.hpp>

#include <memoria/v1/tests/tests.hpp>

namespace memoria {
namespace v1 {
namespace tests {

template <typename T> struct IndirectStateFiledSerializer;


template <typename T>
struct IndirectStateFiledSerializer<InMemAllocator<T>> {
    static void externalize(InMemAllocator<T>& alloc, filesystem::path path, ConfigurationContext* context)
    {
        alloc.store(path);
    }

    static void internalize(InMemAllocator<T>& alloc, filesystem::path path, ConfigurationContext* context)
    {
        alloc = InMemAllocator<T>::load(path);
    }
};

}

namespace YAML {

template <typename T>
struct convert;

template <>
struct convert<UUID> {
    static Node encode(const UUID& rhs) {
        return Node(rhs.to_u8().data());
    }

    static bool decode(const Node& node, UUID& rhs)
    {
        if (!node.IsScalar())
            return false;

        rhs = UUID::parse(node.Scalar().c_str());

        return true;
    }
};

}

}}