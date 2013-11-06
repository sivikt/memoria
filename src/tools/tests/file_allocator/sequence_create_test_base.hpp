
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_FILEALLOC_SEQUENCE_CREATE_TEST_BASE_HPP_
#define MEMORIA_TESTS_FILEALLOC_SEQUENCE_CREATE_TEST_BASE_HPP_

#include <memoria/memoria.hpp>
#include <memoria/tools/tests.hpp>



#include "randomaccesslist_test_base.hpp"

#include <vector>

namespace memoria {

using namespace memoria::vapi;
using namespace std;

template <
    typename ContainerTypeName,
    typename MemBuffer
>
class FASequenceCreateTestBase: public FARandomAccessListTestBase <
                                        ContainerTypeName,
                                        MemBuffer
                              >
{
    typedef FASequenceCreateTestBase<ContainerTypeName, MemBuffer>              MyType;
    typedef MyType                                                              ParamType;


    typedef FARandomAccessListTestBase <
                ContainerTypeName,
                MemBuffer
            >                                                                   Base;

protected:
    typedef typename Base::Ctr                                                  Ctr;
    typedef typename Base::Iterator                                             Iterator;
    typedef typename Base::ID                                                   ID;

public:
    FASequenceCreateTestBase(StringRef name):
        Base(name)
    {}

    virtual void compareBuffers(const MemBuffer& src, const MemBuffer& tgt, const char* source)
    {
        AssertEQ(source, src.size(), tgt.size(), SBuf()<<"buffer sizes are not equal");

        for (size_t c = 0; c < src.size(); c++)
        {
            typename MemBuffer::value_type v1 = src[c];
            typename MemBuffer::value_type v2 = tgt[c];

            AssertEQ(source, v1, v2, [=](){return SBuf()<<"c="<<c;});
        }
    }

    ostream& out() {
        return Base::out();
    }


    void checkIterator(Iterator& iter, const char* source)
    {
        Base::checkIterator(iter, source);
    }
};




}


#endif
