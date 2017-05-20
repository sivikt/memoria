
// Copyright 2016 Victor Smirnov
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

#include "../common/snapshot_base.hpp"

#include <memoria/v1/api/allocator/allocator_inmem_api.hpp>

#include <memoria/v1/core/container/allocator.hpp>
#include <memoria/v1/core/container/ctr_impl.hpp>

#include <memoria/v1/core/exceptions/memoria.hpp>

#include <memoria/v1/core/tools/pool.hpp>
#include <memoria/v1/core/tools/bitmap.hpp>
#include <memoria/v1/core/tools/memory.hpp>
#include <memoria/v1/containers/map/map_factory.hpp>
#include <memoria/v1/core/tools/pair.hpp>
#include <memoria/v1/core/tools/type_name.hpp>



#include <vector>
#include <memory>
#include <mutex>



namespace memoria {
namespace v1 {
namespace persistent_inmem {



template <typename Profile, typename PersistentAllocator>
class Snapshot: public SnapshotBase<Profile, PersistentAllocator, Snapshot<Profile, PersistentAllocator>>
{    
protected:
    using Base = SnapshotBase<Profile, PersistentAllocator, Snapshot<Profile, PersistentAllocator>>;
    using MyType = Snapshot<Profile, PersistentAllocator>;
    
    using typename Base::PersistentAllocatorPtr;
    using typename Base::HistoryNode;
    using typename Base::SnapshotPtr;
    using typename Base::Page;
    
    using AllocatorMutexT	= typename std::remove_reference<decltype(std::declval<HistoryNode>().allocator_mutex())>::type;
    using MutexT			= typename std::remove_reference<decltype(std::declval<HistoryNode>().snapshot_mutex())>::type;
    using StoreMutexT		= typename std::remove_reference<decltype(std::declval<HistoryNode>().store_mutex())>::type;

    using LockGuardT			= typename std::lock_guard<MutexT>;
    using StoreLockGuardT		= typename std::lock_guard<StoreMutexT>;
    using AllocatorLockGuardT	= typename std::lock_guard<AllocatorMutexT>;
    
    
    
    using Base::history_node_;
    using Base::history_tree_;
    using Base::history_tree_raw_;
    using Base::has_open_containers;
    using Base::do_drop;
    using Base::check_tree_structure;


public:
    using Base::uuid;
    
    
    Snapshot(HistoryNode* history_node, const PersistentAllocatorPtr& history_tree):
        Base(history_node, history_tree)
    {}

    Snapshot(HistoryNode* history_node, PersistentAllocator* history_tree):
        Base(history_node, history_tree)
    {
    }
 
    virtual ~Snapshot()
    {
    	//FIXME This code doesn't decrement properly number of active snapshots
    	// for allocator to store data correctly.

    	bool drop1 = false;
    	bool drop2 = false;

    	{
    		LockGuardT snapshot_lock_guard(history_node_->snapshot_mutex());

    		if (history_node_->unref() == 0)
    		{
    			if (history_node_->is_active())
    			{
    				drop1 = true;
    			}
    			else if(history_node_->is_dropped())
    			{
    				drop2 = true;
    				check_tree_structure(history_node_->root());
    			}
    		}
    	}

    	if (drop1)
    	{
    		do_drop();
    		history_tree_raw_->forget_snapshot(history_node_);
    	}

    	if (drop2)
    	{
    		StoreLockGuardT store_lock_guard(history_node_->store_mutex());
    		do_drop();

    		// FIXME: check if absence of snapshot lock here leads to data races...
    	}
    }
    
    

    SnapshotMetadata<UUID> describe() const
    {
    	std::lock(history_node_->snapshot_mutex(), history_node_->allocator_mutex());

    	AllocatorLockGuardT lock_guard2(history_node_->allocator_mutex(), std::adopt_lock);
    	LockGuardT lock_guard1(history_node_->snapshot_mutex(), std::adopt_lock);

    	std::vector<UUID> children;

    	for (const auto& node: history_node_->children())
    	{
    		children.emplace_back(node->txn_id());
    	}

    	auto parent_id = history_node_->parent() ? history_node_->parent()->txn_id() : UUID();

    	return SnapshotMetadata<UUID>(parent_id, history_node_->txn_id(), children, history_node_->metadata(), history_node_->status());
    }

    void commit()
    {
    	LockGuardT lock_guard(history_node_->snapshot_mutex());

        if (history_node_->is_active() || history_node_->is_data_locked())
        {
            history_node_->commit();
            history_tree_raw_->unref_active();
        }
        else {
            throw Exception(MA_SRC, SBuf() << "Invalid state: " << (int32_t)history_node_->status() << " for snapshot " << uuid());
        }
    }

    void drop()
    {
    	std::lock(history_node_->allocator_mutex(), history_node_->snapshot_mutex());

    	AllocatorLockGuardT lock_guard2(history_node_->allocator_mutex(), std::adopt_lock);
    	LockGuardT lock_guard1(history_node_->snapshot_mutex(), std::adopt_lock);

        if (history_node_->parent() != nullptr)
        {
            history_node_->mark_to_clear();
        }
        else {
            throw Exception(MA_SRC, SBuf() << "Can't drop root snapshot " << uuid());
        }
    }

    StringRef metadata() const
    {
    	LockGuardT lock_guard(history_node_->snapshot_mutex());
        return history_node_->metadata();
    }

    void set_metadata(StringRef metadata)
    {
    	LockGuardT lock_guard(history_node_->snapshot_mutex());

        if (history_node_->is_active())
        {
            history_node_->set_metadata(metadata);
        }
        else
        {
            throw Exception(MA_SRC, "Snapshot is already committed.");
        }
    }

    void lock_data_for_import()
    {
    	std::lock(history_node_->allocator_mutex(), history_node_->snapshot_mutex());

    	AllocatorLockGuardT lock_guard2(history_node_->allocator_mutex(), std::adopt_lock);
    	LockGuardT lock_guard1(history_node_->snapshot_mutex(), std::adopt_lock);

    	if (history_node_->is_active())
    	{
    		if (!has_open_containers())
    		{
    			history_node_->lock_data();
    		}
    		else {
    			throw Exception(MA_SRC, SBuf() << "Snapshot " << uuid() << " has open containers");
    		}
    	}
    	else if (history_node_->is_data_locked()) {
    	}
    	else {
    		throw Exception(MA_SRC, SBuf() << "Invalid state: " << (int32_t)history_node_->status() << " for snapshot " << uuid());
    	}
    }


    SnapshotPtr branch()
    {
    	std::lock(history_node_->allocator_mutex(), history_node_->snapshot_mutex());

    	AllocatorLockGuardT lock_guard2(history_node_->allocator_mutex(), std::adopt_lock);
    	LockGuardT lock_guard1(history_node_->snapshot_mutex(), std::adopt_lock);

        if (history_node_->is_committed())
        {
            HistoryNode* history_node = new HistoryNode(history_node_);

            LockGuardT lock_guard3(history_node->snapshot_mutex());

            history_tree_raw_->snapshot_map_[history_node->txn_id()] = history_node;

            return snp_make_shared_init<MyType>(history_node, history_tree_->shared_from_this());
        }
        else if (history_node_->is_data_locked())
        {
        	throw Exception(MA_SRC, SBuf() << "Snapshot " << uuid() << " is locked, branching is not possible.");
        }
        else
        {
            throw Exception(MA_SRC, SBuf() << "Snapshot " << uuid() << " is still being active. Commit it first.");
        }
    }

    bool has_parent() const
    {
    	AllocatorLockGuardT lock_guard(history_node_->allocator_mutex());

        return history_node_->parent() != nullptr;
    }

    SnapshotPtr parent()
    {
    	std::lock(history_node_->snapshot_mutex(), history_node_->allocator_mutex());

    	AllocatorLockGuardT lock_guard2(history_node_->allocator_mutex(), std::adopt_lock);
    	LockGuardT lock_guard1(history_node_->snapshot_mutex(), std::adopt_lock);

        if (history_node_->parent())
        {
            HistoryNode* history_node = history_node_->parent();
            return snp_make_shared_init<MyType>(history_node, history_tree_->shared_from_this());
        }
        else
        {
            throw Exception(MA_SRC, SBuf() << "Snapshot " << uuid() << " has no parent.");
        }
    }

    
    void dump(filesystem::path destination)
    {
    	std::lock(history_node_->snapshot_mutex(), history_node_->allocator_mutex());

    	AllocatorLockGuardT lock_guard2(history_node_->allocator_mutex(), std::adopt_lock);
    	LockGuardT lock_guard1(history_node_->snapshot_mutex(), std::adopt_lock);

    	using Walker = FiberFSDumpContainerWalker<Page>;

    	Walker walker(this->getMetadata(), destination);

    	history_node_->allocator()->build_snapshot_labels_metadata();

    	this->walkContainers(&walker, history_node_->allocator()->get_labels_for(history_node_));

    	history_node_->allocator()->snapshot_labels_metadata().clear();
    }
};

}

template <typename CtrName, typename Profile, typename PersistentAllocator>
auto create(const SnpSharedPtr<persistent_inmem::Snapshot<Profile, PersistentAllocator>>& alloc, const UUID& name)
{
    return alloc->template create<CtrName>(name);
}

template <typename CtrName, typename Profile, typename PersistentAllocator>
auto create(const SnpSharedPtr<persistent_inmem::Snapshot<Profile, PersistentAllocator>>& alloc)
{
    return alloc->template create<CtrName>();
}

template <typename CtrName, typename Profile, typename PersistentAllocator>
auto find_or_create(const SnpSharedPtr<persistent_inmem::Snapshot<Profile, PersistentAllocator>>& alloc, const UUID& name)
{
    return alloc->template find_or_create<CtrName>(name);
}

template <typename CtrName, typename Profile, typename PersistentAllocator>
auto find(const SnpSharedPtr<persistent_inmem::Snapshot<Profile, PersistentAllocator>>& alloc, const UUID& name)
{
    return alloc->template find<CtrName>(name);
}








template <typename Profile>
InMemSnapshot<Profile>::InMemSnapshot() {}


template <typename Profile>
InMemSnapshot<Profile>::InMemSnapshot(SnpSharedPtr<PImpl> impl): pimpl_(impl) {}

template <typename Profile>
InMemSnapshot<Profile>::InMemSnapshot(InMemSnapshot<Profile>&& other): pimpl_(std::move(other.pimpl_)) {}

template <typename Profile>
InMemSnapshot<Profile>::InMemSnapshot(const InMemSnapshot<Profile>&other): pimpl_(other.pimpl_) {}

template <typename Profile>
InMemSnapshot<Profile>& InMemSnapshot<Profile>::operator=(const InMemSnapshot<Profile>& other) 
{
    pimpl_ = other.pimpl_;
    return *this;
}

template <typename Profile>
InMemSnapshot<Profile>& InMemSnapshot<Profile>::operator=(InMemSnapshot<Profile>&& other) 
{
    pimpl_ = std::move(other.pimpl_);
    return *this;
}


template <typename Profile>
InMemSnapshot<Profile>::~InMemSnapshot(){}


template <typename Profile>
bool InMemSnapshot<Profile>::operator==(const InMemSnapshot& other) const
{
    return pimpl_ == other.pimpl_;
}

template <typename Profile>
InMemSnapshot<Profile>::operator bool() const
{
    return pimpl_ != nullptr; 
}




template <typename Profile>
const UUID& InMemSnapshot<Profile>::uuid() const 
{
    return pimpl_->uuid();
}

template <typename Profile>
bool InMemSnapshot<Profile>::is_active() const 
{
    return pimpl_->is_active();
}

template <typename Profile>
bool InMemSnapshot<Profile>::is_marked_to_clear() const 
{
    return pimpl_->is_marked_to_clear();
}

template <typename Profile>
bool InMemSnapshot<Profile>::is_committed() const 
{
    return pimpl_->is_committed();
}

template <typename Profile>
void InMemSnapshot<Profile>::commit() 
{
    return pimpl_->commit();
}

template <typename Profile>
void InMemSnapshot<Profile>::drop() 
{
    return pimpl_->drop();
}

template <typename Profile>
bool InMemSnapshot<Profile>::drop_ctr(const UUID& name) 
{
    return pimpl_->drop_ctr(name);
}

template <typename Profile>
void InMemSnapshot<Profile>::set_as_master() 
{
    return pimpl_->set_as_master();
}

template <typename Profile>
void InMemSnapshot<Profile>::set_as_branch(StringRef name) 
{
    return pimpl_->set_as_branch(name);
}

template <typename Profile>
StringRef InMemSnapshot<Profile>::snapshot_metadata() const 
{
    return pimpl_->metadata();
}

template <typename Profile>
void InMemSnapshot<Profile>::set_snapshot_metadata(StringRef metadata) 
{
    return pimpl_->set_metadata(metadata);
}

template <typename Profile>
void InMemSnapshot<Profile>::lock_data_for_import() 
{
    return pimpl_->lock_data_for_import();
}

template <typename Profile>
typename InMemSnapshot<Profile>::SnapshotPtr InMemSnapshot<Profile>::branch() 
{
    return pimpl_->branch();
}

template <typename Profile>
bool InMemSnapshot<Profile>::has_parent() const 
{
    return pimpl_->has_parent();
}

template <typename Profile>
typename InMemSnapshot<Profile>::SnapshotPtr InMemSnapshot<Profile>::parent() 
{
    return pimpl_->parent();
}

template <typename Profile>
void InMemSnapshot<Profile>::import_new_ctr_from(InMemSnapshot<Profile>& txn, const UUID& name) 
{
    return pimpl_->import_new_ctr_from(txn.pimpl_, name);
}

template <typename Profile>
void InMemSnapshot<Profile>::copy_new_ctr_from(InMemSnapshot<Profile>& txn, const UUID& name) 
{
    return pimpl_->copy_new_ctr_from(txn.pimpl_, name);
}

template <typename Profile>
void InMemSnapshot<Profile>::import_ctr_from(InMemSnapshot<Profile>& txn, const UUID& name) 
{
    return pimpl_->import_ctr_from(txn.pimpl_, name);
}

template <typename Profile>
void InMemSnapshot<Profile>::copy_ctr_from(InMemSnapshot<Profile>& txn, const UUID& name) 
{
    return pimpl_->copy_ctr_from(txn.pimpl_, name);
}

template <typename Profile>
bool InMemSnapshot<Profile>::check() {
    return pimpl_->check();
}

template <typename Profile>
void InMemSnapshot<Profile>::dump(filesystem::path destination) {
    return pimpl_->dump(destination);
}

template <typename Profile>
void InMemSnapshot<Profile>::dump_persistent_tree() 
{
    return pimpl_->dump_persistent_tree();
}

template <typename Profile>
void InMemSnapshot<Profile>::walk_containers(ContainerWalker* walker, const char* allocator_descr) 
{
     return pimpl_->walkContainers(walker, allocator_descr);
}

template <typename Profile>
void InMemSnapshot<Profile>::reset() 
{
    return pimpl_.reset();
}

template <typename Profile>
ContainerMetadataRepository* InMemSnapshot<Profile>::metadata() const
{
    return pimpl_->getMetadata();
}


template <typename Profile>
SnpSharedPtr<IAllocator<ProfilePageType<Profile>>> InMemSnapshot<Profile>::snapshot_ref_creation_allowed() 
{
    pimpl_->checkIfConainersCreationAllowed();
    return static_pointer_cast<IAllocator<ProfilePageType<Profile>>>(pimpl_->shared_from_this());
}


template <typename Profile>
SnpSharedPtr<IAllocator<ProfilePageType<Profile>>> InMemSnapshot<Profile>::snapshot_ref_opening_allowed() 
{
    pimpl_->checkIfConainersOpeneingAllowed();
    return static_pointer_cast<IAllocator<ProfilePageType<Profile>>>(pimpl_->shared_from_this());
}


template <typename Profile>
Logger& InMemSnapshot<Profile>::logger()
{
    return pimpl_->logger();
}


template <typename Profile>
CtrRef<Profile> InMemSnapshot<Profile>::get(const UUID& name) 
{
    return CtrRef<Profile>(pimpl_->get(name));
}

}}