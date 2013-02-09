
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_SYMBOLSEQ_SYMSEQ_TEST_SUITE_HPP_
#define MEMORIA_TESTS_SYMBOLSEQ_SYMSEQ_TEST_SUITE_HPP_

#include "../tests_inc.hpp"

#include "symseq_misc_test.hpp"

#include <vector>

namespace memoria {

using namespace memoria::vapi;
using namespace std;

class SymbolSeqTestSuite: public TestSuite {

public:

    SymbolSeqTestSuite(): TestSuite("SymbolSeqSuite")
    {
    	registerTask(new SymSeqMiscTest<1>());
//    	registerTask(new SymSeqMiscTest<2>());
//    	registerTask(new SymSeqMiscTest<3>());
    }

};

}


#endif
