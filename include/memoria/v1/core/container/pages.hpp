
// Copyright 2011 Victor Smirnov
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

#include <memoria/v1/core/types/typelist.hpp>
#include <memoria/v1/core/container/builder.hpp>

#include <memoria/v1/core/tools/bitmap.hpp>
#include <memoria/v1/core/tools/buffer.hpp>
#include <memoria/v1/core/tools/reflection.hpp>
#include <memoria/v1/core/types/typehash.hpp>
#include <memoria/v1/core/tools/reflection.hpp>
#include <memoria/v1/core/tools/id.hpp>
#include <memoria/v1/core/tools/uuid.hpp>

#include <memoria/v1/core/exceptions/exceptions.hpp>

namespace memoria {
namespace v1 {

struct Block {

    virtual UUID getId() const                       = 0;
    virtual uint64_t getContainerHash() const        = 0;
    virtual uint64_t getPageTypeHash() const         = 0;
    virtual int64_t getFlags() const                 = 0;
    virtual const void* Ptr() const                  = 0;
    virtual void* Ptr()                              = 0;
    virtual void setPtr(void* ptr)                   = 0;
    virtual bool isNull() const                      = 0;

    virtual int32_t size() const                     = 0;
    virtual int32_t getByte(int32_t idx) const       = 0;
    virtual void setByte(int32_t idx, int32_t value) = 0;

    virtual ~Block() noexcept {}
};



template <typename BlockType>
class BlockWrapper: public Block {
    BlockType *page_;
public:
    BlockWrapper(BlockType* page): page_(page) {}
    BlockWrapper(): page_(NULL) {}

    virtual ~BlockWrapper() noexcept  {}

    virtual bool isNull() const {
        return page_ == NULL;
    }

    virtual UUID getId() const
    {
        if (page_)
        {
            return page_->id();
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }

    virtual uint64_t getContainerHash() const
    {
        if (page_)
        {
            return page_->ctr_type_hash();
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }

    virtual uint64_t getPageTypeHash() const
    {
        if (page_)
        {
            return page_->page_type_hash();
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }
    
    virtual int64_t getFlags() const {
        return 0;
    }
    
    virtual void* Ptr() {
        return page_;
    }

    virtual const void* Ptr() const {
        return page_;
    }

    virtual void setPtr(void* ptr)
    {
        page_ = static_cast<BlockType*>(ptr);
    }

    virtual int32_t size() const {
        return page_->page_size();
    }

    virtual int32_t getByte(int32_t idx) const
    {
        if (page_ != NULL)
        {
            if (idx >= 0 && idx < page_->page_size()) {
                return T2T<uint8_t*>(page_)[idx];
            }
            else {
                MMA1_THROW(BoundsException()) << WhatInfo(fmt::format8(u"Invalid byte offset: {} max={}", idx, page_->page_size()));
            }
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }

    virtual void setByte(int32_t idx, int32_t value)
    {
        if (page_ != NULL)
        {
            if (idx >= 0 && idx < page_->page_size())
            {
                T2T<uint8_t*>(page_)[idx] = (uint8_t)value;
            }
            else {
                MMA1_THROW(BoundsException()) << WhatInfo(fmt::format8(u"Invalid byte offset: {} max={}", idx, page_->page_size()));
            }
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }
};


template <typename BlockType>
class BlockWrapper<const BlockType>: public Block {
    const BlockType *page_;
public:
    BlockWrapper(const BlockType* page): page_(page) {}
    BlockWrapper(): page_(NULL) {}

    virtual ~BlockWrapper() noexcept  {}

    virtual bool isNull() const {
        return page_ == NULL;
    }

    virtual UUID getId() const
    {
        if (page_ != NULL)
        {
            return page_->id();
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }

    virtual uint64_t getContainerHash() const
    {
        if (page_ != NULL)
        {
            return page_->ctr_type_hash();
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }

    virtual uint64_t getPageTypeHash() const
    {
        if (page_ != NULL)
        {
            return page_->page_type_hash();
        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }

    virtual int64_t getFlags() const {
        return 0;
    }

    virtual void* Ptr() {
        MMA1_THROW(Exception()) << WhatCInfo("Page in not mutable");
    }

    virtual const void* Ptr() const {
        return page_;
    }

    virtual void setPtr(void* ptr)
    {
        MMA1_THROW(Exception()) << WhatCInfo("Page in not mutable");
    }

    virtual int32_t size() const {
        return page_->page_size();
    }

    virtual int32_t getByte(int32_t idx) const
    {
        if (page_ != NULL)
        {
            if (idx >= 0 && idx < page_->page_size()) {
                return T2T<uint8_t*>(page_)[idx];
            }
            else {
                MMA1_THROW(BoundsException()) << WhatInfo(fmt::format8(u"Invalid byte offset: {} max={}", idx, page_->page_size()));
            }

        }
        else {
            MMA1_THROW(NullPointerException()) << WhatCInfo("Page data is not set");
        }
    }

    virtual void setByte(int32_t idx, int32_t value)
    {
        MMA1_THROW(Exception()) << WhatCInfo("Page in not mutable");
    }
};



template <typename Name, typename Base>
class PagePart;

template <typename Name>
class PagePartNotFound;


class EmptyPart{};




template <
        typename PartsList,
        typename Base
>
struct PageBuilder: public Builder<PartsList, PagePart, Base> {};


template <int Idx, typename Types>
class PageHelper: public PagePart<
                    SelectByIndex<Idx, typename Types::List>,
                    PageHelper<Idx - 1, Types>
                  >
{
};

template <typename Types>
class PageHelper<-1, Types>: public Types::NodePageBase {

};


template <typename Types>
class PageStart: public PageHelper<ListSize<typename Types::List> - 1, Types> {

};

}}

#include <memoria/v1/core/container/page_traits.hpp>
