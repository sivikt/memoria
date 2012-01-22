
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CORE_API_METADATA_MODEL_HPP
#define _MEMORIA_CORE_API_METADATA_MODEL_HPP

#include <memoria/metadata/model.hpp>
#include <memoria/metadata/page.hpp>
#include <memoria/core/vapi/metadata/group.hpp>



namespace memoria { namespace vapi {

template <typename Interface>
class ContainerMetadataImplT: public MetadataGroupImplT<Interface> {
	typedef ContainerMetadataImplT<Interface> 		Me;
	typedef MetadataGroupImplT<Interface> 		Base;
public:

	ContainerMetadataImplT(StringRef name, const MetadataList &content, Int code, ContainerFactoryFn factory): Base(name, content), code_(code), factory_(factory), hash_(code_) {
		Base::set_type() = Interface::MODEL;
		for (UInt c = 0; c < content.size(); c++)
	    {
	        if (content[c]->GetTypeCode() == Metadata::PAGE)
	        {
	            PageMetadata *page = static_cast<PageMetadata*> (content[c]);
	            page_map_[page->Hash()] = page;
	            hash_ += page->Hash() + code;
	        }
	        else {
	            //exception;
	        }
	    }
	}

	virtual ~ContainerMetadataImplT() throw () {}

	virtual Int Hash() const {
		return hash_;
	}

	virtual Int Code() const {
		return code_;
	}

//	virtual ContainerFactoryFn Factory() const {
//		return factory_;
//	}

	virtual PageMetadata* GetPageMetadata(Int hashCode) const {
		PageMetadataMap::const_iterator i = page_map_.find(hashCode);
		if (i != page_map_.end()) {
			return i->second;
		}
		else {
			throw MemoriaException(MEMORIA_SOURCE, "Unknown page type hash code");
		}
	}

private:

    PageMetadataMap     page_map_;

    Int code_;
    ContainerFactoryFn 		factory_;
    Int hash_;
};


typedef ContainerMetadataImplT<ContainerMetadata> 					ContainerMetadataImpl;



}}


#endif