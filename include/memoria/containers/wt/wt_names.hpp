
// Copyright 2013 Victor Smirnov
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

namespace memoria {
namespace wt {

template <typename... LabelDescriptors>
class WTLabeledTree {};

class CtrApiName    {};
class CtrCTreeName  {};
class CtrInsertName {};
class CtrToolsName  {};
class CtrRemoveName {};
class CtrChecksName {};
class CtrUpdateName {};
class CtrFindName {};

class ItrApiName {};
}

template <typename Types>
struct WTCtrTypesT: CtrTypesT<Types> {};

template <typename Types>
struct WTIterTypesT: IterTypesT<Types> {};



template <typename Types>
using WTCtrTypes  = WTCtrTypesT<Types>;

template <typename Types>
using WTIterTypes = WTIterTypesT<Types>;

}