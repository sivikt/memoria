
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_PACKED_C_LOUDS_SUITE_HPP_
#define MEMORIA_TESTS_PACKED_C_LOUDS_SUITE_HPP_

#include "../../tests_inc.hpp"


#include "packed_lcardinal_test.hpp"

namespace memoria {


class PackedLoudsCardinalTestSuite: public TestSuite {

public:

    PackedLoudsCardinalTestSuite(): TestSuite("Packed.LoudsCardinalSuite")
    {
        registerTask(new PackedLoudsCardinalTest());
    }
};

}


#endif
