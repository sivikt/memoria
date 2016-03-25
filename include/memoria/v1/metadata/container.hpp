
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

#include <memoria/v1/metadata/group.hpp>
#include <memoria/v1/metadata/page.hpp>
#include <memoria/v1/core/exceptions/exceptions.hpp>
#include <memoria/v1/core/tools/file.hpp>
#include <memoria/v1/core/tools/assert.hpp>
#include <memoria/v1/core/tools/platform.hpp>
#include <memoria/v1/core/tools/uuid.hpp>

#include <stack>
#include <sstream>
#include <fstream>
#include <ostream>

namespace memoria {
namespace v1 {


struct ContainerWalker {
    virtual void beginAllocator(const char* type, const char* desc)             = 0;
    virtual void endAllocator()                                                 = 0;

    virtual void beginSnapshot(const char* descr)                               = 0;
    virtual void endSnapshot()                                                  = 0;

    virtual void beginSnapshotSet(const char* descr, size_t number)             = 0;
    virtual void endSnapshotSet()                                               = 0;

    virtual void beginCompositeCtr(const char* descr, const UUID& name)         = 0;
    virtual void endCompositeCtr()                                              = 0;

    virtual void beginCtr(const char* descr, const UUID& name, const UUID& root)= 0;
    virtual void endCtr()                                                       = 0;

    virtual void beginRoot(Int idx, const void* page)                           = 0;
    virtual void endRoot()                                                      = 0;

    virtual void beginNode(Int idx, const void* page)                           = 0;
    virtual void endNode()                                                      = 0;

    virtual void rootLeaf(Int idx, const void* page)                            = 0;
    virtual void leaf(Int idx, const void* page)                                = 0;

    virtual void singleNode(const char* descr, const void* page)                = 0;

    virtual void beginSection(const char* name)                                 = 0;
    virtual void endSection()                                                   = 0;

    virtual void content(const char* name, const char* content)                 = 0;

    virtual ~ContainerWalker() {}
};




struct ContainerInterface {
    virtual bool check(const UUID& root_id, const UUID& name, void* allocator) const = 0;
    virtual void walk(
            const UUID& root_id,
            const UUID& name,
            void* allocator,
            ContainerWalker* walker
    ) const                                                                     = 0;

    virtual String ctr_type_name() const                                        = 0;

    virtual void drop(
            const UUID& root_id,
            const UUID& name,
            void* allocator
    )                                                                           = 0;


    virtual ~ContainerInterface() {}
};

struct ContainerMetadata: public MetadataGroup {
public:

    ContainerMetadata(StringRef name, const MetadataList &content, Int ctr_hash, ContainerInterface* container_interface):
        MetadataGroup(name, content),
        container_interface_(container_interface),
        ctr_hash_(ctr_hash)
    {
        MetadataGroup::set_type() = MetadataGroup::CONTAINER;
        for (UInt c = 0; c < content.size(); c++)
        {
            if (content[c]->getTypeCode() == Metadata::PAGE)
            {
                PageMetadata *page = static_cast<PageMetadata*> (content[c]);
                page_map_[page->hash() ^ ctr_hash] = page;
            }
            else if (content[c]->getTypeCode() == Metadata::CONTAINER) {
                // nothing to do
            }
            else {
                //exception;
            }
        }
    }

    virtual ~ContainerMetadata() throw ()
    {
        delete container_interface_;
    }

    virtual Int ctr_hash() const {
        return ctr_hash_;
    }

    virtual PageMetadata* getPageMetadata(Int model_hash, Int page_hash) const
    {
        PageMetadataMap::const_iterator i = page_map_.find(model_hash ^ page_hash);
        if (i != page_map_.end())
        {
            return i->second;
        }
        else {
            throw Exception(MEMORIA_SOURCE, "Unknown page type hash code");
        }
    }

    virtual ContainerInterface* getCtrInterface() const
    {
        return container_interface_;
    }

private:

    PageMetadataMap         page_map_;
    ContainerInterface*     container_interface_;

    Int                     ctr_hash_;
};



struct ContainerMetadataRepository: public MetadataGroup {

public:

    ContainerMetadataRepository(StringRef name, const MetadataList &content);

    virtual ~ContainerMetadataRepository() throw ()
    {
        //FIXME need to rewrite ownership for metadata objects
        for (auto entry: model_map_)
        {
            delete entry.second;
        }
    }

    virtual Int hash() const {
        return hash_;
    }

    PageMetadata* getPageMetadata(Int model_hash, Int page_hash) const;
    ContainerMetadata* getContainerMetadata(Int model_hash) const;


    virtual void registerMetadata(ContainerMetadata* metadata)
    {
        process_model(metadata);
    }

    virtual void unregisterMetadata(ContainerMetadata* metadata) {}

    void dumpMetadata(std::ostream& out);

private:
    Int                     hash_;
    PageMetadataMap         page_map_;
    ContainerMetadataMap    model_map_;

    void process_model(ContainerMetadata* model);
};








template <typename PageType>
class FSDumpContainerWalker: public ContainerWalker {

    typedef PageType                                                            Page;
    typedef typename Page::ID                                                   ID;

    ContainerMetadataRepository* metadata_;
    std::stack<File> path_;

public:
    FSDumpContainerWalker(ContainerMetadataRepository* metadata, StringRef root):
        metadata_(metadata)
    {
        File root_path(root);

        if (!root_path.isExists())
        {
            root_path.mkDirs();
        }
        else {
            root_path.delTree();
            root_path.mkDirs();
        }

        path_.push(root_path);
    }

    virtual void beginSnapshotSet(const char* descr, size_t number)
    {
        pushFolder(descr);
    }

    virtual void endSnapshotSet()
    {
        path_.pop();
    }

    virtual void beginAllocator(const char* type, const char* desc)
    {
        pushFolder(type);
    }

    virtual void endAllocator()
    {
        path_.pop();
    }

    virtual void beginSnapshot(const char* descr)
    {
        pushFolder(descr);
    }

    virtual void endSnapshot()
    {
        path_.pop();
    }

    virtual void beginCompositeCtr(const char* descr, const UUID& name)
    {
        stringstream str;

        str<<name;

        pushFolder(str.str().c_str());

        dumpDescription("ctr_name", String(descr));
    }

    virtual void endCompositeCtr() {
        path_.pop();
    }

    virtual void beginCtr(const char* descr, const UUID& name, const UUID& root)
    {
        stringstream str;

        str<<shorten(descr)<<": "<<name;

        pushFolder(str.str().c_str());

        dumpDescription("ctr_name", String(descr));
    }

    virtual void endCtr() {
        path_.pop();
    }

    virtual void rootLeaf(Int idx, const void* page_data)
    {
        const Page* page = T2T<Page*>(page_data);

        String file_name = path_.top().getPath() + Platform::getFilePathSeparator() + "root_leaf.txt";

        dumpPage(file_name, page);
    }

    virtual void leaf(Int idx, const void* page_data)
    {
        const Page* page = T2T<Page*>(page_data);

        String description = getNodeName("Leaf", idx, page->id());

        String file_name = path_.top().getPath() + Platform::getFilePathSeparator() + description + ".txt";

        dumpPage(file_name, page);
    }

    virtual void beginRoot(Int idx, const void* page_data)
    {
        beginNonLeaf("Root", idx, page_data);
    }

    virtual void endRoot()
    {
        path_.pop();
    }

    virtual void beginNode(Int idx, const void* page_data)
    {
        beginNonLeaf("Node", idx, page_data);
    }

    virtual void endNode()
    {
        path_.pop();
    }

    virtual void singleNode(const char* description, const void* page_data)
    {
        const Page* page = T2T<Page*>(page_data);

        String file_name = path_.top().getPath() + Platform::getFilePathSeparator() + description + ".txt";

        dumpPage(file_name, page);
    }


    virtual void beginSection(const char* name)
    {
        pushFolder(name);
    }

    virtual void endSection() {
        path_.pop();
    }

    virtual void content(const char* name, const char* content)
    {
        dumpDescription(name, content);
    }

private:

    void beginNonLeaf(const char* type, Int idx, const void* page_data)
    {
        const Page* page = T2T<Page*>(page_data);

        String folder_name = getNodeName(type, idx, page->id());
        pushFolder(folder_name.c_str());

        String file_name = path_.top().getPath() + Platform::getFilePathSeparator() + "0_page.txt";

        dumpPage(file_name, page);
    }


    void dumpPage(StringRef file, const Page* page)
    {
        std::ofstream pagetxt(file.c_str());

        PageMetadata* meta = metadata_->getPageMetadata(page->ctr_type_hash(), page->page_type_hash());

        dumpPageData(meta, page, pagetxt);
    }

    void dumpDescription(StringRef type, StringRef content)
    {
        String file_name = path_.top().getPath() + Platform::getFilePathSeparator() + type + ".txt";

        std::ofstream file(file_name.c_str());

        file<<content;
    }

    void pushFolder(const char* descr)
    {
        String name = path_.top().getPath() + Platform::getFilePathSeparator() + String(descr);
        File file(name);
        MEMORIA_V1_ASSERT_TRUE(file.mkDir());
        path_.push(file);
    }

    String getNodeName(const char* name, Int index, const ID& id)
    {
        std::stringstream str;

        str<<name<<"-";

        char prev = str.fill();

        str.fill('0');
        str.width(4);

        str<<index;

        str.fill(prev);

        str<<"___"<<id;

        return str.str();
    }

private:
    String shorten(const char* txt)
    {
        String text = txt;

        auto start = text.find_first_of("<");

        if (start != String::npos)
        {
            text.erase(start);
        }

        return text;
    }
};


//template <typename Allocator>
//void FSDumpAllocator(Allocator* allocator, StringRef path)
//{
//    typedef FSDumpContainerWalker<typename Allocator::Page> Walker;
//
//    Walker walker(allocator->getMetadata(), path);
//    allocator->walkContainers(&walker);
//}

template <typename Allocator>
void FSDumpAllocator(const std::shared_ptr<Allocator>& allocator, StringRef path)
{
    typedef FSDumpContainerWalker<typename Allocator::Page> Walker;

    Walker walker(allocator->getMetadata(), path);
    allocator->walkContainers(&walker);
}

}}