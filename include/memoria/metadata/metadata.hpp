
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_VAPI_METADATA_METADATA_HPP1
#define _MEMORIA_VAPI_METADATA_METADATA_HPP1

#include <memoria/core/types/types.hpp>

#include <memoria/core/types/traits.hpp>
#include <memoria/core/types/typelist.hpp>
#include <memoria/core/types/typemap.hpp>
#include <memoria/core/tools/bitmap.hpp>

#include <memoria/core/tools/id.hpp>
#include <memoria/core/tools/config.hpp>

#include <vector>
#include <map>
#include <string>

namespace memoria    {
namespace vapi       {

struct Metadata;
struct PageMetadata;
struct ContainerMetadata;
struct FieldMetadata;
struct ContainerCollection;
struct Container;

typedef std::vector<Metadata*>          MetadataList;
typedef std::map<Int, PageMetadata*>    PageMetadataMap;
typedef std::map<Int, ContainerMetadata*>   ContainerMetadataMap;
typedef std::map<Int, FieldMetadata*>   PtrMap;

typedef Container* (*ContainerFactoryFn) (const IDValue& rootId, ContainerCollection *container, BigInt name);
typedef Int (*PageSizeProviderFn)(const void *page);

struct MEMORIA_API Metadata: public Typed {
    enum   {BYTE,   UBYTE,  SHORT,   USHORT, INT,    UINT,
            BIGINT, ID,     BITMAP,  FLAG,   GROUP,  PAGE,
            MODEL,  CONTAINER};

    virtual StringRef Name() const  = 0;
    virtual Int GetTypeCode() const = 0;
    virtual bool IsGroup() const    = 0;
    virtual bool IsField() const    = 0;
    virtual bool IsNumber() const    = 0;

    
};


struct MEMORIA_API Page: public Typed {

    virtual IDValue GetId() const                    = 0;
    virtual Int GetContainerHash() const                 = 0;
    virtual Int GetPageTypeHash() const              = 0;
    virtual BigInt GetFlags() const                  = 0;
    virtual const void* Ptr() const                  = 0;
    virtual void* Ptr()                  		 	 = 0;
    virtual void SetPtr(void* ptr)              	 = 0;
    virtual bool IsNull() const						 = 0;

    virtual Int Size() const                         = 0;
    virtual Int GetByte(Int idx) const               = 0;
    virtual void SetByte(Int idx, Int value)    	 = 0;

    
};



void InitMetadataTypes();
void DestroyMetadataTypes();

}}

#endif
