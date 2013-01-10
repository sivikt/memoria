
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TESTS_BIT_VECTOR_BITMAP_MISC_TEST_HPP_
#define MEMORIA_TESTS_BIT_VECTOR_BITMAP_MISC_TEST_HPP_

#include <memoria/memoria.hpp>
#include <memoria/tools/tests.hpp>
#include <memoria/core/tools/bitmap_select.hpp>

#include "bitmap_test_base.hpp"

#include <vector>
#include <limits>
#include <functional>

namespace memoria {

using namespace memoria::vapi;
using namespace std;

template <typename T>
class BitmapMiscTest: public BitmapTestBase<T> {
    typedef BitmapMiscTest<T>                                                  	MyType;
    typedef BitmapTestBase<T>                                                   Base;



public:
    BitmapMiscTest(StringRef name):
        Base(name)
    {
        MEMORIA_ADD_TEST(testMakeMaskInternal);
        MEMORIA_ADD_TEST(testMakeMask);
        MEMORIA_ADD_TEST(testSetBit);
        MEMORIA_ADD_TEST(testSetBits);
    }

    template <typename TT>
    TT makeMask(int start, int length)
    {
    	TT value = 0;
    	TT one = 1;

    	for (Int c = 0; c < length; c++)
    	{
    		value = value | (one << (start + c));
    	}

    	return value;
    }

    void testMakeMaskInternal(ostream& out)
    {
    	UInt value = 0xF;

    	for (Int c = 0; c < 32 - 5; c++, value <<= 1)
    	{
    		AssertEQ(MA_SRC, makeMask<UInt>(c, 4), value);
    	}
    }

    void testMakeMask(ostream& out)
    {
    	out<<"MakeMask "<<TypeNameFactory<T>::name()<<endl;

    	Int bitsize = TypeBitsize<T>();

    	for (Int size = 0; size < bitsize; size++)
    	{
    		for (Int pos = 0; pos < bitsize - size; pos++)
    		{
    			T mask1 = MakeMask<T>(pos, size);
    			T mask2 = makeMask<T>(pos, size);

    			AssertEQ(MA_SRC, mask1, mask2, SBuf()<<size<<" "<<pos);
    		}
    	}
    }

    void testSetBit(ostream& out)
    {
    	out<<"TestSetBit: "<<TypeNameFactory<T>::name()<<endl;

    	T values[4];
    	Int bitsize = sizeof(values) * 8;

    	for (Int c = 0; c < bitsize; c++)
    	{
    		for (size_t d = 0; d < sizeof(values)/sizeof(T); d++)
    		{
    			values[d] = 0;
    		}

    		SetBit(values, c, 1);
    		Int value = GetBit(values, c);

    		AssertEQ(MA_SRC, value, 1);

    		for (size_t d = 0; d < sizeof(values)/sizeof(T); d++)
    		{
    			values[d] = static_cast<T>(-1);
    		}

			SetBit(values, c, 0);
			value = GetBit(values, c);

			AssertEQ(MA_SRC, value, 0, SBuf()<<c);
    	}
    }


    void testSetBits(ostream& out)
    {
    	out<<"TestSetBits: "<<TypeNameFactory<T>::name()<<endl;

    	T values[4];

    	Int bitsize 	= sizeof(values) * 8;
    	Int atomsize	= sizeof(T) * 8;

    	for (Int length = 1; length <= atomsize; length++)
    	{
    		for (Int start = 0; start < bitsize - length; start++)
    		{
    			for (size_t d = 0; d < sizeof(values)/sizeof(T); d++)
    			{
    				values[d] = sizeof(T) == 8 ? getBIRandom() : getRandom();
    			}

    			T bits1 = (sizeof(T) == 8 ? getBIRandom() : getRandom()) & MakeMask<T>(0, length);

    			SetBits(values, start, bits1, length);

    			for (Int c = 0; c < length; c++)
    			{
    				Int v1 = GetBit(values, start + c);
    				Int v2 = (bits1 >> c) & 0x1;

    				AssertEQ(MA_SRC, v1, v2, SBuf()<<(start + c));
    			}

    			T bits2 = GetBits(values, start, length);

    			for (Int c = 0; c < length; c++)
    			{
    				Int v1 = GetBit(values, start + c);
    				Int v2 = (bits2 >> c) & 0x1;

    				AssertEQ(MA_SRC, v1, v2, SBuf()<<(start + c));
    			}
    		}
    	}

    	for (Int c = 0; c < bitsize; c++)
    	{
    		for (size_t d = 0; d < sizeof(values)/sizeof(T); d++)
    		{
    			values[d] = 0;
    		}

    		SetBit(values, c, 1);
    		Int value = GetBit(values, c);

    		AssertEQ(MA_SRC, value, 1);

    		for (size_t d = 0; d < sizeof(values)/sizeof(T); d++)
    		{
    			values[d] = static_cast<T>(-1);
    		}

    		SetBit(values, c, 0);
    		value = GetBit(values, c);

    		AssertEQ(MA_SRC, value, 0, SBuf()<<c);
    	}
    }

    void testMoveBits(ostream& out)
    {
    	T bitmap1[5];
    	T bitmap2[5];

    	Int bitsize 	= sizeof(bitmap1) * 8;

    	for (Int length = 0; length < bitsize; length++)
    	{
    		makeRandomBitmap(bitmap1, bitsize);

    		out<<"length: "<<length<<endl;

    		for (Int src_bit = 0; src_bit < bitsize - length; src_bit++)
    		{
    			for (Int dst_bit = 0; dst_bit < bitsize - length; dst_bit++)
    			{
    				MoveBitsFW(bitmap1, bitmap2, src_bit, dst_bit, length);

    				compareBitmaps(MA_SRC, bitmap1, bitmap2, src_bit, dst_bit, length);

    				clearBitmap(bitmap2, bitsize);

    				MoveBitsBW(bitmap1, bitmap2, src_bit, dst_bit, length);

    				compareBitmaps(MA_SRC, bitmap1, bitmap2, src_bit, dst_bit, length);
    			}
    		}
    	}
    }
};


}
#endif