
// Copyright 2018 Victor Smirnov
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.


#pragma once

#include <memoria/v1/core/config.hpp>
#include <memoria/v1/core/types/type2type.hpp>
#include <memoria/v1/core/tools/config.hpp>

#include <memoria/v1/core/exceptions/base.hpp>

#include <unicode/ustring.h>
#include <unicode/utypes.h>

#include <string.h>

#include <utility>

namespace memoria {
namespace v1 {

struct UStringSizeTag {};

using ICUErrorInfo = boost::error_info<struct TagICUErrorInfo, const char*>;

class StringException: public ::memoria::v1::MemoriaThrowable {
    UErrorCode code_;
public:
    StringException(UErrorCode code):
        code_(code)
    {}

    UErrorCode code() const {
        return code_;
    }

    static void assertOk(const UErrorCode& code)
    {
        if (U_FAILURE(code))
        {
            MMA1_THROW(StringException(code)) << ICUErrorInfo(u_errorName(code));
        }
    }
};


namespace _ {
struct UErrorCodeStatus {
    UErrorCode status_ {U_ZERO_ERROR};

    ~UErrorCodeStatus() {
        StringException::assertOk(status_);
    }
};
}

template <typename Fn>
auto with_icu_error(Fn&& fn) {
    _::UErrorCodeStatus status;
    return fn(&status.status_);
}

static inline char* castChars(char* ptr) {return ptr;}
static inline const char* castChars(const char* ptr) {return ptr;}

static inline wchar_t* castChars(wchar_t* ptr) {return ptr;}
static inline const wchar_t* castChars(const wchar_t* ptr) {return ptr;}

static inline char16_t* castChars(UChar* ptr) {return T2T<char16_t*>(ptr);}
static inline const char16_t* castChars(const UChar* ptr) {return T2T<const char16_t*>(ptr);}

static inline UChar32* castChars(char32_t* ptr) {return T2T<UChar32*>(ptr);}
static inline const UChar32* castChars(const char32_t* ptr) {return T2T<const UChar32*>(ptr);}

static inline int32_t UStrUTF8Length(const char* str, int32_t size = -1)
{
    UErrorCode code = U_ZERO_ERROR;
    int32_t len{};
    u_strFromUTF8(nullptr, 0, &len, str, size, &code);

    if (code != U_BUFFER_OVERFLOW_ERROR) {
        StringException::assertOk(code);
    }

    return len;
}

static inline int32_t UStrUTF8Length(const char16_t* str, int32_t size = -1)
{
    UErrorCode code = U_ZERO_ERROR;
    int32_t len{};
    u_strToUTF8(nullptr, 0, &len, castChars(str), size, &code);

    if (code != U_BUFFER_OVERFLOW_ERROR) {
        StringException::assertOk(code);
    }

    return len;
}


static inline int32_t UStrUTF32Length(const char32_t* str, int32_t size = -1)
{
    UErrorCode code = U_ZERO_ERROR;
    int32_t len{};
    u_strFromUTF32(nullptr, 0, &len, castChars(str), size, &code);

    if (code != U_BUFFER_OVERFLOW_ERROR) {
        StringException::assertOk(code);
    }

    return len;
}


static inline int32_t UStrUTF32Length(const char16_t* str, int32_t size = -1)
{
    UErrorCode code = U_ZERO_ERROR;
    int32_t len{};
    u_strToUTF32(nullptr, 0, &len, castChars(str), size, &code);

    if (code != U_BUFFER_OVERFLOW_ERROR) {
        StringException::assertOk(code);
    }

    return len;
}

static inline int32_t UStrUWLength(const wchar_t* str, int32_t size = -1)
{
    UErrorCode code = U_ZERO_ERROR;
    int32_t len{};
    u_strFromWCS(nullptr, 0, &len, castChars(str), size, &code);

    if (code != U_BUFFER_OVERFLOW_ERROR) {
        StringException::assertOk(code);
    }

    return len;
}

static inline int32_t UStrUWLength(const char16_t* str, int32_t size = -1)
{
    UErrorCode code = U_ZERO_ERROR;
    int32_t len{};
    u_strToWCS(nullptr, 0, &len, castChars(str), size, &code);

    if (code != U_BUFFER_OVERFLOW_ERROR) {
        StringException::assertOk(code);
    }

    return len;
}



}}
