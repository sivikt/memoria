
// Copyright Victor Smirnov 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_ALLOCATORS_PERSISTENT_INMEM_PERSISTENT_TREE_TXN_HPP
#define _MEMORIA_ALLOCATORS_PERSISTENT_INMEM_PERSISTENT_TREE_TXN_HPP

#include <memoria/allocators/persistent-inmem/persistent_tree_node.hpp>

#include <memoria/core/container/allocator.hpp>
#include <memoria/core/container/metadata_repository.hpp>

#include <memoria/core/exceptions/memoria.hpp>

#include <memoria/core/tools/pool.hpp>
#include <memoria/core/tools/bitmap.hpp>

#include <memoria/containers/root/root_factory.hpp>

#include <vector>
#include <memory>

namespace memoria {
namespace persistent_inmem {




template <typename Profile, typename PageType, typename HistoryNode, typename PersitentTree, typename HistoryTree>
class Snapshot: public IWalkableAllocator<PageType> {
	using Base 				= IAllocator<PageType>;
	using MyType 			= Snapshot;

	using HistoryTreePtr 	= std::shared_ptr<HistoryTree>;
	using SnapshotPtr 		= std::shared_ptr<Snapshot>;

	using Status 			= typename HistoryNode::Status;
public:

	using typename Base::Page;
	using typename Base::ID;
	using typename Base::PageG;
	using typename Base::Shared;
	using typename Base::AbstractAllocator;
	using typename Base::CtrShared;

private:
	using RootMapType 	= Ctr<typename CtrTF<Profile, Root>::CtrTypes>;
	using CtrSharedMap 	= std::unordered_map<BigInt, CtrShared*>;

	class Properties: public IAllocatorProperties {
	public:
		virtual Int defaultPageSize() const
		{
			return 4096;
		}

		virtual BigInt lastCommitId() const {
			return 0;
		}

		virtual void setLastCommitId(BigInt txn_id) {}

		virtual BigInt newTxnId() {return 0;}
	};

	CtrSharedMap ctr_shared_;

	HistoryNode* 	history_node_;
	HistoryTreePtr  history_tree_;
	HistoryTree*  	history_tree_raw_ = nullptr;

	PersitentTree persistent_tree_;

	StaticPool<ID, Shared, 256>  pool_;

	Logger logger_;

	Properties properties_;

	std::unique_ptr<RootMapType> root_map_;


	ContainerMetadataRepository*  metadata_;

	template <typename, typename>
	friend class PersistentInMemAllocatorT;

	template <typename T>
	friend struct std::allocator_traits;

public:

	Snapshot(HistoryNode* history_node, const HistoryTreePtr& history_tree):
		history_node_(history_node),
		history_tree_(history_tree),
		history_tree_raw_(history_tree.get()),
		persistent_tree_(history_node_),
		logger_("PersistentInMemAllocatorTxn"),
		root_map_(nullptr),
		metadata_(MetadataRepository<Profile>::getMetadata())
	{
		root_map_ = std::make_unique<RootMapType>(this, history_node->is_active() ? CTR_CREATE : CTR_FIND, 0);
	}

	Snapshot(HistoryNode* history_node, HistoryTree* history_tree):
		history_node_(history_node),
		history_tree_raw_(history_tree),
		persistent_tree_(history_node_),
		logger_("PersistentInMemAllocatorTxn"),
		root_map_(nullptr),
		metadata_(MetadataRepository<Profile>::getMetadata())
	{
		root_map_ = std::make_unique<RootMapType>(this, history_node->is_active() ? CTR_CREATE : CTR_FIND, 0);
	}

	virtual ~Snapshot()
	{
		// FIXME: autocommit ?
		history_node_->commit();
	}

	static void initMetadata() {
		RootMapType::initMetadata();
	}

	ContainerMetadataRepository* getMetadata() const {
		return metadata_;
	}

	void freeze()
	{
		if (history_node_->is_active())
		{
			history_node_->commit();
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Invalid Snapshot state: "<<(Int)history_node_->status());
		}
	}

	void clear()
	{
		if (history_node_->is_active())
		{
			// do clear
		}
		else if (history_node_->is_committed())
		{
			throw vapi::Exception(MA_SRC, "Snapshot has been already freezed");
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Invalid Snapshot state: "<<(Int)history_node_->status());
		}
	}


	void set_as_master()
	{
		history_tree_raw_->set_master(history_node_->txn_id());
	}

	void set_as_branch(StringRef name)
	{
		history_tree_raw_->set_branch(name, history_node_->txn_id());
	}

	StringRef metadata() const {
		return history_node_->metadata();
	}

	void set_metadata(StringRef metadata)
	{
		if (history_node_->is_active())
		{
			history_node_->metadata() = metadata;
		}
		else
		{
			throw vapi::Exception(MA_SRC, "Snapshot is already freezed.");
		}
	}

	SnapshotPtr branch()
	{
		if (history_node_->is_committed())
		{
			HistoryNode* history_node = new HistoryNode(history_node_);
			history_tree_raw_->snapshot_map_[history_node->txn_id()] = history_node;

			return std::make_shared<Snapshot>(history_node, history_tree_);
		}
		else
		{
			throw vapi::Exception(MA_SRC, "Snapshot is still being active. Freeze it first.");
		}
	}

	void join(SnapshotPtr txn) {}

	virtual PageG getPage(const ID& id, BigInt name)
	{
		if (id.isSet())
		{
			Shared* shared = get_shared(id, Shared::READ);

			if (!shared->get())
			{
				auto page_opt = persistent_tree_.find(id);

				if (page_opt)
				{
					const auto& txn_id = history_node_->txn_id();

					if (page_opt.value().txn_id() != txn_id)
					{
						shared->state() = Shared::READ;
					}
					else {
						shared->state() = Shared::UPDATE;
					}

					shared->set_page(page_opt.value().page());
				}
				else {
					throw vapi::Exception(MA_SRC, SBuf()<<"Page is not found for the specified id: "<<id);
				}
			}

			return PageG(shared);
		}
		else {
			return PageG();
		}
	}

	virtual PageG getPageForUpdate(const ID& id, BigInt name)
	{
		MEMORIA_ASSERT_TRUE(history_node_->is_active());

		if (id.isSet())
		{
			Shared* shared = get_shared(id, Shared::UPDATE);

			if (!shared->get())
			{
				auto page_opt = persistent_tree_.find(id);

				if (page_opt)
				{
					const auto& txn_id = history_node_->txn_id();

					if (page_opt.value().txn_id() != txn_id)
					{
						Page* new_page = clone_page(page_opt.value().page());
						shared->set_page(new_page);

						shared->refresh();
					}
					else {
						MEMORIA_ASSERT(shared->state(), ==, Shared::UPDATE);

						shared->set_page(page_opt.value().page());

						shared->refresh();
					}
				}
				else {
					throw vapi::Exception(MA_SRC, SBuf()<<"Page is not found for the specified id: "<<id);
				}
			}
			else if (shared->state() == Shared::READ)
			{
				auto page_opt = persistent_tree_.find(id);

				if (page_opt)
				{
					Page* new_page = clone_page(page_opt.value().page());

					set_page(new_page);
					shared->set_page(new_page);

					shared->refresh();
				}
				else {
					throw vapi::Exception(MA_SRC, SBuf()<<"Page is not found for the specified id: "<<id);
				}
			}
			else if (shared->state() == Shared::UPDATE)
			{
				//MEMORIA_ASEERT();
			}
			else {
				throw vapi::Exception(MA_SRC, SBuf()<<"Invalid PageShared state: "<<shared->state());
			}

			shared->state() = Shared::UPDATE;

			return PageG(shared);
		}
		else {
			return PageG();
		}
	}



	virtual PageG updatePage(Shared* shared, BigInt name)
	{
		MEMORIA_ASSERT_TRUE(history_node_->is_active());

        if (shared->state() == Shared::READ)
        {
        	Page* new_page = clone_page(shared->get());

        	set_page(new_page);

        	shared->set_page(new_page);

            shared->state() = Shared::UPDATE;

            shared->refresh();
        }

        return PageG(shared);
	}

	virtual void removePage(const ID& id, BigInt name)
	{
		MEMORIA_ASSERT_TRUE(history_node_->is_active());

		auto iter = persistent_tree_.locate(id);

		if (!iter.is_end())
		{
			auto shared = pool_.get(id);

			if (!shared)
			{
				persistent_tree_.remove(iter);
			}
			else {
				shared->state() = Shared::DELETE;
			}
		}
	}

	virtual PageG createPage(Int initial_size, BigInt name)
	{
		MEMORIA_ASSERT_TRUE(history_node_->is_active());

		if (initial_size == -1)
		{
			initial_size = properties_.defaultPageSize();
		}

		void* buf = this->malloc(initial_size);

		memset(buf, 0, initial_size);

		ID id = newId();

		Page* p = new (buf) Page(id);

		p->page_size() = initial_size;

		Shared* shared  = pool_.allocate(id);

		shared->id()    = id;
		shared->state()	= Shared::UPDATE;

		shared->set_page(p);
		shared->set_allocator(this);

		set_page(p);



		return PageG(shared);
	}


	virtual void resizePage(Shared* shared, Int new_size)
	{
		MEMORIA_ASSERT_TRUE(history_node_->is_active());

		if (shared->state() == Shared::READ)
		{
			Page* page = shared->get();
			PageMetadata* pageMetadata = metadata_->getPageMetadata(page->ctr_type_hash(), page->page_type_hash());

			Page* new_page = T2T<Page*>(this->malloc(new_size));

			pageMetadata->getPageOperations()->resize(page, new_page, new_size);

			shared->set_page(new_page);

			set_page(new_page);
		}
		else if (shared->state() == Shared::UPDATE)
		{
			Page* page = shared->get();
			PageMetadata* pageMetadata = metadata_->getPageMetadata(page->ctr_type_hash(), page->page_type_hash());

			Page* new_page  = T2T<Page*>(realloc(page, new_size));

			pageMetadata->getPageOperations()->resize(page, new_page, new_size);

			shared->set_page(new_page);

			set_page(new_page);
		}
	}

	virtual void releasePage(Shared* shared) noexcept
	{
		if (shared->state() == Shared::DELETE)
		{
			persistent_tree_.remove(shared->get()->id());
		}

		pool_.release(shared->id());
	}

	virtual PageG getPageG(Page* page)
	{
		throw vapi::Exception(MA_SRC, "Method getPageG is not implemented for this allocator");
	}

	virtual CtrShared* getCtrShared(BigInt name)
	{
        auto i = ctr_shared_.find(name);

        if (i != ctr_shared_.end())
        {
            return i->second;
        }
        else
        {
            throw vapi::Exception(MEMORIA_SOURCE, SBuf()<<"Unknown CtrShared requested for name "<<name);
        }
	}

	virtual bool isCtrSharedRegistered(BigInt name)
	{
		return ctr_shared_.find(name) != ctr_shared_.end();
	}

	virtual void unregisterCtrShared(CtrShared* shared)
	{
		ctr_shared_.erase(shared->name());
	}

	virtual void registerCtrShared(CtrShared* shared)
	{
        auto name = shared->name();

        auto i = ctr_shared_.find(name);

        if (i == ctr_shared_.end())
        {
            ctr_shared_[name] = shared;
        }
        else if (i->second == NULL)
        {
            i->second = shared;
        }
        else
        {
            throw Exception(MEMORIA_SOURCE, SBuf()<<"CtrShared for name "<<name<<" is already registered");
        }
	}

	virtual ID newId() {
		return history_tree_raw_->newId();
	}

	virtual BigInt currentTxnId() const {
		return 0;
	}

	// memory pool allocator

	virtual void* allocateMemory(size_t size) {
		return ::malloc(size);
	}
	virtual void  freeMemory(void* ptr) {
		::free(ptr);
	}

	virtual Logger& logger() {return logger_;}
	virtual IAllocatorProperties& properties() {
		return properties_;
	}

	virtual ID getRootID(BigInt name) {
		if (name == 0)
		{
			return root();
		}
		else {
			return this->get_value_for_key(name);
		}
	}

	virtual void setRoot(BigInt name, const ID& root)
	{
		new_root(name, root);
	}

	virtual void markUpdated(BigInt name) {}

	virtual bool hasRoot(BigInt name)
	{
		if (root_map_)
		{
			return get_value_for_key(name) != ID(0);
		}
		else {
			return false;
		}
	}

	virtual BigInt createCtrName()
	{
        auto meta = root_map_->getRootMetadata();

        BigInt new_name = ++meta.model_name_counter();

        root_map_->setRootMetadata(meta);

        return new_name;
	}



	virtual void flush(bool force_sync = false) {}
	virtual void rollback(bool force_sync = false) {}

	virtual bool check()
	{
		bool result = false;

		for (auto iter = root_map_->Begin(); !iter.isEnd(); )
		{
			BigInt ctr_name = iter.key();

			PageG page = this->getPage(iter.value(), ctr_name);

			ContainerMetadata* ctr_meta = metadata_->getContainerMetadata(page->ctr_type_hash());

			result = ctr_meta->getCtrInterface()->check(&page->id(), ctr_name, this) || result;

			iter++;
		}

		return result;
	}

	virtual void walkContainers(vapi::ContainerWalker* walker, const char* allocator_descr = nullptr)
	{
        walker->beginSnapshot((SBuf()<<"Snapshot-"<<history_node_->txn_id()).str().c_str());

        auto iter = root_map_->Begin();

        while (!iter.isEnd())
        {
            auto ctr_name   = iter.key();
            auto root_id    = iter.value();

            auto page       = this->getPage(root_id, ctr_name);

            Int master_hash = page->master_ctr_type_hash();
            Int ctr_hash    = page->ctr_type_hash();

            ContainerMetadata* ctr_meta = metadata_->getContainerMetadata(master_hash != 0 ? master_hash : ctr_hash);

            ctr_meta->getCtrInterface()->walk(&page->id(), ctr_name, this, walker);

            iter++;
        }

        walker->endSnapshot();
	}

protected:

	Page* clone_page(const Page* page)
	{
		char* buffer = (char*) this->malloc(page->page_size());

		CopyByteBuffer(page, buffer, page->page_size());
		Page* new_page = T2T<Page*>(buffer);

		new_page->uuid() 		= newId();
		new_page->references() 	= 0;

		return new_page;
	}

	Shared* get_shared(Page* page)
	{
		MEMORIA_ASSERT_TRUE(page != nullptr);

		Shared* shared = pool_.get(page->id());

		if (shared == NULL)
		{
			shared = pool_.allocate(page->id());

			shared->id()        = page->id();
			shared->state()     = Shared::UNDEFINED;
			shared->set_page(page);
			shared->set_allocator(this);
		}

		return shared;
	}

	Shared* get_shared(const ID& id, Int state)
	{
		Shared* shared = pool_.get(id);

		if (shared == NULL)
		{
			shared = pool_.allocate(id);

			shared->id()        = id;
			shared->state()     = state;
			shared->set_page((Page*)nullptr);
			shared->set_allocator(this);
		}

		return shared;
	}

    void* malloc(size_t size)
    {
    	return ::malloc(size);
    }

    void set_page(Page* page)
    {
    	page->ref();

    	const auto& txn_id = history_node_->txn_id();
    	using Value = typename PersitentTree::Value;
    	persistent_tree_.assign(page->id(), Value(page, txn_id));
    }

    void set_root(BigInt name, const ID &page_id)
    {
    	if (name != 0)
    	{
    		this->set_value_for_key(name, page_id);
    	}
    	else {
    		history_node_->root_id() = page_id;
    	}
    }


    void remove_by_key(BigInt name)
    {
        root_map_->remove(name);
    }

    void set_value_for_key(BigInt name, const ID& page_id)
    {
    	auto iter = root_map_->find(name);

    	if (iter.isFound(name))
    	{
    		iter.setValue(page_id);
    	}
    	else {
    		iter.insert(name, page_id);
    	}

        root_map_->find(name).setValue(page_id);
    }

    ID get_value_for_key(BigInt name)
    {
    	if (name != 0)
    	{
    		auto iter = root_map_->find(name);

    		if (!iter.isEnd())
    		{
    			return iter.value();
    		}
    		else {
    			return ID(0);
    		}
    	}
    	else {
    		return history_node_->root_id();
    	}
    }

    virtual void new_root(BigInt name, const ID &page_id)
    {
    	if (page_id.isNull())
    	{
    		remove_root(name);
    	}
    	else {
    		set_root(name, page_id);
    	}
    }

    void remove_root(BigInt name)
    {
    	if (name != 0)
    	{
    		this->remove_by_key(name);
    	}
    	else {
    		throw vapi::Exception(MA_SRC, SBuf()<<"Allocator directory removal attempted");
    	}
    }



    const ID &root() const
    {
        if (root_map_ != nullptr)
        {
            return root_map_->root();
        }
        else
        {
            return history_node_->root_id();
        }
    }
};


}
}



#endif