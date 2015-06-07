
// Copyright Victor Smirnov 2014.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#include <typeinfo>
#include <iostream>

#include <memoria/core/types/types.hpp>
#include <memoria/core/tools/type_name.hpp>
#include <memoria/core/types/list/append.hpp>
#include <memoria/core/types/algo/select.hpp>
#include <memoria/core/types/static_md5.hpp>

using namespace std;
using namespace memoria;
using namespace memoria::vapi;

typedef ValueList<UInt, 23234, 452345, 34545345, 3452345> UArray1;


using namespace memoria::md5;


int main(void) {

    typedef ValueList<UInt, 1,3,7,2, 9,2,17,25, 81,43,767,12, 4351,3,7,55,
                            66,77,0,345, 23423,234,45345,45345, 34,23245,2344,56767, 34, 2, 67, 4098,
                            123, 445, 678> List;

    cout<<TypeNameFactory<List>::name()<<endl<<endl;

//    cout<<TypeNameFactory<Sublist<0, List>::Type>::name()<<endl;
//    cout<<TypeNameFactory<Sublist<1, List>::Type>::name()<<endl;
//    cout<<TypeNameFactory<Sublist<2, List>::Type>::name()<<endl;
//    cout<<TypeNameFactory<Sublist<3, List>::Type>::name()<<endl;

//    Md5Sum<List>::dump();

    cout<<Md5Sum<List>::Result::Value<<endl;

    return 0;
}

