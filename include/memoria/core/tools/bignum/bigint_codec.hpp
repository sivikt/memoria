
// Copyright Victor Smirnov 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_CORE_TOOLS_BIGINT_CODEC_HPP_
#define MEMORIA_CORE_TOOLS_BIGINT_CODEC_HPP_


#include <memoria/core/types/types.hpp>

#include <memoria/core/tools/bignum/bigint.hpp>

namespace memoria {



template <size_t FixedPartPrecision >
class IntegerCodec<BigIntegerT<FixedPartPrecision>> {
public:
    using BufferType 	= UByte;
    using T 			= BufferType;
    using V 			= BigIntegerT<FixedPartPrecision>;

    static const Int BitsPerOffset  = 16;
    static const Int ElementSize    = 8; // In bits;

    size_t length(const T* buffer, size_t idx, size_t limit) const
    {
    	auto head = buffer[idx];
    	if (head < 252) {
    		return 1;
    	}
    	else if (head < 254) {
    		return buffer[idx + 1] + 1;
    	}
    	else {
    		size_t len = 0;
    		for (size_t c = idx + 1; c < idx + 4; c++)
    		{
    			len |= buffer[c];
    			len <<= 8;
    		}

    		return len + 5;
    	}
    }

    size_t length(const V& value) const
    {
        if (value.is_small())
        {
        	auto digits = value.content_.fixed_.digits_;

        	Int msb = value.msb();

        	if (msb < 32)
        	{
        		if (!value.is_negative())
        		{
        			if (*digits < 126ul)
        			{
        				return 1;
        			}
        			else {
        				return 2 + (msb >> 3);
        			}
        		}
        		else {
        			if (*digits < 127ul)
        			{
        				return 1;
        			}
        			else {
        				return 2 + (msb >> 3);
        			}
        		}
        	}
        	else {
        		return 2 + (msb >> 3);
        	}
        }
        else {
        	auto digits = value.content_.variable_.digits_;

        	Int msb = value.msb();

        	if (msb < 256)
        	{
        		return 2 + (msb >> 3);
        	}
        	else {
        		return 5 + (msb >> 3);
        	}
        }
    }

    size_t decode(const T* buffer, V& value, size_t idx, size_t limit) const
    {
        return decode(buffer, value, idx);
    }

    size_t decode(const T* buffer, V& value, size_t idx) const
    {
    	auto header = buffer[idx];

    	if (header < 126u)
    	{
    		value.assign(header);
    	}
    	else if (header < 252u)
    	{
    		value.assign(-(header - 125));
    	}
    	else if (header < 254u)
    	{
    		idx++;

    		size_t len = buffer[idx++];

    		if (len <= 16)
    		{
    			value.make_small();
    		}
    		else {
    			value.make_big(len);
    		}

    		value.set_negative(header == 253u);

    		auto data = value.data();

    		deserialize(buffer, data, idx, len);

    		value.set_msb();

    		return len + 2;
    	}
    	else {
    		idx++;

    		size_t len = 0;

    		for (UInt c = 0; c < 4; c++)
    		{
    			len |= ((size_t)buffer[idx++]) << (c << 2);
    		}

    		value.make_big(len);

    		value.set_negative(header == 255u);

    		auto data = value.data();

    		deserialize(buffer, data, idx, len);

    		value.compute_msb();

    		return len + 5;
    	}

        return 0;
    }

    size_t encode(T* buffer, const V& value, size_t idx, size_t limit) const
    {
        return encode(buffer, value, idx);
    }

    size_t encode(T* buffer, const V& value, size_t idx) const
    {
    	if (value.is_small())
    	{
    		auto digits = value.content_.fixed_.digits_;

    		UInt msb = value.msb();

    		if (msb < 32)
    		{
    			auto v = *digits;
    			if (!value.is_negative())
    			{
    				if (v < 126u)
    				{
    					buffer[idx] = v;
    					return 1;
    				}
    				else
    				{
    					size_t len = (msb >> 3);

    					buffer[idx] = 252u;
    					buffer[idx + 1] = len;

    					for (size_t c = idx + 2; c < idx + 2 + len; c++)
    					{
    						buffer[c] = v;
    						v >>= 8;
    					}

    					return 2 + len;
    				}
    			}
    			else {
    				if (v < 127ul)
    				{
    					buffer[idx] = v + 125;
    					return 1;
    				}
    				else
    				{
    					size_t len = (msb >> 3);

    					buffer[idx] = 253u;
    					buffer[idx + 1] = len;

    					for (size_t c = idx + 2; c < idx + 2 + len; c++)
    					{
    						buffer[c] = v;
    						v >>= 8;
    					}

    					return 2 + len;
    				}
    			}
    		}
    		else {
    			UInt len = (msb >> 3);

    			buffer[idx] = (!value.is_negative()) ? 252u : 253u;
    			buffer[idx + 1] = len;

    			serialize(buffer, digits, idx + 2, len);

    			return 2 + len;
    		}
    	}
    	else {
    		auto digits = value.content_.variable_.digits_;

    		UInt msb = value.msb();
    		size_t len = msb >> 3;

    		if (msb < 256u)
    		{
    			buffer[idx] = (!value.is_negative()) ? 254u : 255u;
    			buffer[idx + 1] = len;

    			serialize(buffer, digits, idx + 2, len);

    			return 2 + len;
    		}
    		else {
    			buffer[idx++] = (!value.is_negative()) ? 254u : 255u;

    			for (UInt c = 0; c < 4; c++)
    			{
    				buffer[idx++] = len >> (c << 3);
    			}

    			serialize(buffer, digits, idx, len);

    			return 5 + len;
    		}
    	}
    }

    void move(T* buffer, size_t from, size_t to, size_t size) const
    {
        CopyBuffer(buffer + from, buffer + to, size);
    }

    void copy(const T* src, size_t from, T* tgt, size_t to, size_t size) const
    {
        CopyBuffer(src + from, tgt + to, size);
    }

private:

    template <typename D>
    void serialize(T* buffer, const D* digits, size_t idx, size_t len)
    {
    	for (size_t c = 0; c < len; c++)
    	{
    		buffer[idx++] = digits[c >> 2] >> (c & 0x3);
    	}
    }

    template <typename D>
    void deserialize(const T* buffer, D* digits, size_t idx, size_t len)
    {
    	for (size_t c = 0; c < len; c++)
    	{
    		digits[c >> 2] = ((D)buffer[idx++]) << (c & 0x2);
    	}
    }

};



}

#endif
