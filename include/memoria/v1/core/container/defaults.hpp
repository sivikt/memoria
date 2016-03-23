
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memoria/v1/core/container/names.hpp>
#include <memoria/v1/core/container/pages.hpp>

#include <memoria/v1/core/container/allocator.hpp>

namespace memoria {
namespace v1 {


class AbstractTransaction {
public:
    AbstractTransaction() {}
};


template <
    typename Profile, typename IDValueType = BigInt, int FlagsCount = 32, typename TransactionType = AbstractTransaction
>
struct BasicContainerCollectionCfg {

    typedef UUID                                                                ID;
    typedef AbstractPage <ID, FlagsCount>                                       Page;
    typedef TransactionType                                                     Transaction;

    typedef IAllocator<Page>                                                    AbstractAllocator;

    typedef AbstractAllocator                                                   AllocatorType;
};


}}