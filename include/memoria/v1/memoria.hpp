// Copyright 2011 Victor Smirnov
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

#include <memoria/v1/allocators/persistent-inmem/factory.hpp>
#include <memoria/v1/core/container/metadata_repository.hpp>
#include <memoria/v1/core/tools/file.hpp>

namespace memoria {
namespace v1 {

template <>
struct CtrNameDeclarator<0>: TypeDef<Map<UUID, UUID>> {};

}}