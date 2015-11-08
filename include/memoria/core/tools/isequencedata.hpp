
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CORE_TOOLS_ISEQUENCEDATA_HPP
#define _MEMORIA_CORE_TOOLS_ISEQUENCEDATA_HPP


namespace memoria    {
namespace vapi       {


template <typename T, Int BitsPerSymbol>
struct ISequenceDataSource {
    virtual ~ISequenceDataSource() throw() {}
};


template <typename T, Int BitsPerSymbol>
struct ISequenceDataTarget {
    virtual ~ISequenceDataTarget() throw() {}
};


}
}



#endif
