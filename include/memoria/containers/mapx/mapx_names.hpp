
// Copyright Victor Smirnov 2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_MODELS_IDX_MAPX_NAMES_HPP
#define _MEMORIA_MODELS_IDX_MAPX_NAMES_HPP

#include <memoria/prototypes/bt/bt_names.hpp>

namespace memoria    {
namespace mapx       {

class CtrApiName        {};
class CtrCApiName       {};
class CtrInsertName     {};
class CtrCInsertName    {};
class CtrCNavName       {};
class CtrRemoveName     {};
class CtrCRemoveName    {};


class ItrApiName {};
class ItrCApiName {};
class ItrNavName {};
class ItrCNavName {};

class ItrValueName {};
class ItrCValueName {};
class ItrMrkValueName {};

}

template <typename Types>
struct MapXCtrTypesT: CtrTypesT<Types> {};

template <typename Types>
struct MapXIterTypesT: IterTypesT<Types> {};



template <typename Types>
using MapXCtrTypes  = BTCtrTypes<MapXCtrTypesT<Types>>;

template <typename Types>
using MapXIterTypes = BTIterTypes<MapXIterTypesT<Types>>;


}

#endif