
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_CONTAINERS_WT_TOOLS_HPP
#define _MEMORIA_CONTAINERS_WT_TOOLS_HPP

#include <memoria/prototypes/bt/bt_tools.hpp>
#include <memoria/containers/wt/wt_names.hpp>

#include <memoria/core/types/typehash.hpp>

namespace memoria       {
namespace wt            {
}


template <typename... LabelDescriptors>
struct TypeHash<wt::WTLabeledTree<LabelDescriptors...>>: TypeHash<LabeledTree<LabelDescriptors...>> {};

template <typename... LabelDescriptors>
struct TypeHash<wt::WTLabeledTree<TypeList<LabelDescriptors...>>>: TypeHash<LabeledTree<LabelDescriptors...>> {};





}

#endif
