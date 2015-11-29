
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_PACKED_ARRAY_TEST_SUITE_HPP_
#define MEMORIA_TESTS_PACKED_ARRAY_TEST_SUITE_HPP_

#include "../../tests_inc.hpp"


#include "packed_array_misc_test.hpp"

namespace memoria {

using namespace memoria::vapi;
using namespace std;

class PackedArrayTestSuite: public TestSuite {

public:

    PackedArrayTestSuite(): TestSuite("Packed.Array")
    {
        registerTask(new PackedArrayMiscTest<PkdVDArray<BigInt, 1, UByteExintCodec>>("Misc.VLD.Exint"));
        registerTask(new PackedArrayMiscTest<PkdVDArray<BigInt, 1, UBigIntEliasCodec>>("Misc.VLD.Elias"));
    }

};

}


#endif

