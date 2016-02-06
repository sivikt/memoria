
// Copyright Victor Smirnov 2016.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)


#ifndef _MEMORIA_ALLOCATORS_PERSISTENT_INMEM_ALLOCATOR_HPP
#define _MEMORIA_ALLOCATORS_PERSISTENT_INMEM_ALLOCATOR_HPP

#include <unordered_map>
#include <string>



#include <memoria/core/container/metadata_repository.hpp>

#include <memoria/core/tools/pool.hpp>
#include <memoria/core/tools/uuid.hpp>
#include <memoria/core/tools/stream.hpp>

#include <memoria/allocators/persistent-inmem/persistent_tree_node.hpp>
#include <memoria/allocators/persistent-inmem/persistent_tree.hpp>

#include <memoria/allocators/inmem/factory.hpp>

#include <malloc.h>
#include <memoria/allocators/persistent-inmem/persistent_tree_snapshot.hpp>
#include <memory>
#include <limits>
#include <unordered_map>

namespace memoria {

template <typename ValueT, typename TxnIdT>
class PersistentTreeValue {
	ValueT page_;
	TxnIdT txn_id_;
public:
	PersistentTreeValue() {}
	PersistentTreeValue(const ValueT& page, const TxnIdT& txn_id): page_(page), txn_id_(txn_id) {}

	const ValueT& page() const {return page_;}
	ValueT& page() {return page_;}

	const TxnIdT& txn_id() const {return txn_id_;}
	TxnIdT& txn_id() {return txn_id_;}
};

template <typename V, typename T>
vapi::OutputStreamHandler& operator<<(vapi::OutputStreamHandler& out, const PersistentTreeValue<V, T>& value)
{
    out << value.page();
    out << value.txn_id();
    return out;
}

template <typename V, typename T>
vapi::InputStreamHandler& operator>>(vapi::InputStreamHandler& in, PersistentTreeValue<V, T>& value)
{
	in >> value.page();
	in >> value.txn_id();
    return in;
}

template <typename V, typename T>
std::ostream& operator<<(std::ostream& out, const PersistentTreeValue<V, T>& value)
{
	out<<"PersistentTreeValue[";
    out << value.page();
    out<<", ";
    out << value.txn_id();
    out<<"]";

    return out;
}


template <typename Profile, typename PageType>
class PersistentInMemAllocatorT: public enable_shared_from_this<PersistentInMemAllocatorT<Profile, PageType>> {
public:

	static constexpr Int NodeIndexSize 	= 32;
	static constexpr Int NodeSize 		= NodeIndexSize * 8;

	using MyType 		= PersistentInMemAllocatorT<Profile, PageType>;

	using Page 			= PageType;


	using Key 			= typename PageType::ID;
	using Value 		= PageType*;

	using TxnId 		 	= UUID;
	using PTreeNodeId 	 	= UUID;

	using LeafNodeT 		= memoria::persistent_inmem::LeafNode<Key, PersistentTreeValue<PageType*, TxnId>, NodeSize, NodeIndexSize, PTreeNodeId, TxnId>;
	using LeafNodeBufferT 	= memoria::persistent_inmem::LeafNode<Key, PersistentTreeValue<typename PageType::ID, TxnId>, NodeSize, NodeIndexSize, PTreeNodeId, TxnId>;

	using BranchNodeT 		= memoria::persistent_inmem::BranchNode<Key, NodeSize, NodeIndexSize, PTreeNodeId, TxnId>;
	using BranchNodeBufferT = memoria::persistent_inmem::BranchNode<Key, NodeSize, NodeIndexSize, PTreeNodeId, TxnId, PTreeNodeId>;
	using NodeBaseT 		= typename BranchNodeT::NodeBaseT;
	using NodeBaseBufferT 	= typename BranchNodeBufferT::NodeBaseT;



	struct HistoryNode {

		enum class Status {ACTIVE, COMMITTED, DROPPED};

	private:
		HistoryNode* parent_;
		String metadata_;

		std::vector<HistoryNode*> children_;

		NodeBaseT* root_;

		typename PageType::ID root_id_;

		Status status_;

		TxnId txn_id_;

		BigInt references_ = 0;

	public:

		HistoryNode(HistoryNode* parent, Status status = Status::ACTIVE):
			parent_(parent),
			root_(nullptr),
			root_id_(),
			status_(status),
			txn_id_(UUID::make_random())
		{
			if (parent_) {
				parent_->children().push_back(this);
			}
		}

		HistoryNode(const TxnId& txn_id, HistoryNode* parent, Status status):
			parent_(parent),
			root_(nullptr),
			root_id_(),
			status_(status),
			txn_id_(txn_id)
		{
			if (parent_) {
				parent_->children().push_back(this);
			}
		}

		void remove_child(HistoryNode* child)
		{
			for (size_t c = 0; c < children_.size(); c++)
			{
				if (children_[c] == child)
				{
					children_.erase(children_.begin() + c);
					break;
				}
			}
		}

		void remove_from_parent() {
			if (parent_) {
				parent_->remove_child(this);
			}
		}

		bool is_committed() const {
			return status_ == Status::COMMITTED;
		}

		bool is_active() const {
			return status_ == Status::ACTIVE;
		}

		bool is_dropped() const {
			return status_ == Status::DROPPED;
		}

		const TxnId& txn_id() const {return txn_id_;}

		const auto& root() const {return root_;}

		auto root() {return root_;}

		auto& root_id() {return root_id_;}
		const auto& root_id() const {return root_id_;}

		void set_root(NodeBaseT* new_root) {
			root_ = new_root;
		}

		PTreeNodeId new_node_id() {
			return UUID::make_random();
		}

		const auto& parent() const {return parent_;}
		auto& parent() {return parent_;}

		auto& children() {
			return children_;
		}

		const auto& children() const {
			return children_;
		}


		Status& status() {return status_;}
		const Status& status() const {return status_;}

		auto& metadata() {return metadata_;}
		const auto& metadata() const {return metadata_;}

		void commit() {
			status_ = Status::COMMITTED;
		}

		void mark_to_clear() {
			status_ = Status::DROPPED;
		}

		auto references() {return references_;}
		auto ref() {
			return ++references_;
		}

		auto unref() {
			return --references_;
		}
	};

	struct HistoryNodeBuffer {

		template <typename, typename>
		friend class PersistentInMemAllocatorT;

	private:
		TxnId parent_;

		String metadata_;

		std::vector<TxnId> children_;

		PTreeNodeId root_;
		typename PageType::ID root_id_;

		typename HistoryNode::Status status_;

		TxnId txn_id_;

	public:

		HistoryNodeBuffer(){}

		const TxnId& txn_id() const {
			return txn_id_;
		}

		TxnId& txn_id() {
			return txn_id_;
		}

		const auto& root() const {return root_;}

		auto& root() {return root_;}

		auto& root_id() {return root_id_;}
		const auto& root_id() const {return root_id_;}

		const auto& parent() const {return parent_;}
		auto& parent() {return parent_;}

		auto& children() {
			return children_;
		}

		const auto& children() const {
			return children_;
		}

		auto& status() {return status_;}
		const auto& status() const {return status_;}

		auto& metadata() {return metadata_;}
		const auto& metadata() const {return metadata_;}
	};


	using PersistentTreeT 	= memoria::persistent_inmem::PersistentTree<BranchNodeT, LeafNodeT, HistoryNode, PageType>;
	using SnapshotT 	 	= memoria::persistent_inmem::Snapshot<Profile, PageType, HistoryNode, PersistentTreeT, MyType>;
	using SnapshotPtr 		= std::shared_ptr<SnapshotT>;

	using TxnMap		 		= std::unordered_map<TxnId, HistoryNode*, UUIDKeyHash, UUIDKeyEq>;

	using HistoryTreeNodeMap	= std::unordered_map<PTreeNodeId, HistoryNodeBuffer*, UUIDKeyHash, UUIDKeyEq>;
	using PersistentTreeNodeMap	= std::unordered_map<PTreeNodeId, std::pair<NodeBaseBufferT*, NodeBaseT*>, UUIDKeyHash, UUIDKeyEq>;
	using PageMap				= std::unordered_map<typename PageType::ID, PageType*, UUIDKeyHash, UUIDKeyEq>;
	using BranchMap				= std::unordered_map<String, HistoryNode*>;

	template <typename, typename, typename, typename, typename>
	friend class memoria::persistent_inmem::Snapshot;

private:

	class AllocatorMetadata {
		TxnId master_;
		TxnId root_;

		std::unordered_map<String, TxnId> named_branches_;

	public:
		AllocatorMetadata() {}

		TxnId& master() {return master_;}
		TxnId& root() {return root_;}

		const TxnId& master() const {return master_;}
		const TxnId& root()   const {return root_;}

		auto& named_branches() {return named_branches_;}
		const auto& named_branches() const {return named_branches_;}
	};

	class Checksum {
		BigInt records_;
	public:
		BigInt& records() {return records_;}
		const BigInt& records() const {return records_;}
	};

	enum {TYPE_UNKNOWN, TYPE_METADATA, TYPE_HISTORY_NODE, TYPE_BRANCH_NODE, TYPE_LEAF_NODE, TYPE_DATA_PAGE, TYPE_CHECKSUM};

	HistoryNode* history_tree_ = nullptr;
	HistoryNode* master_ = nullptr;

	TxnMap snapshot_map_;

	BranchMap named_branches_;

	ContainerMetadataRepository*  metadata_;

	BigInt records_ = 0;

	BigInt active_snapshots_ = 0;

	PersistentInMemAllocatorT():
		metadata_(MetadataRepository<Profile>::getMetadata())
	{
		SnapshotT::initMetadata();

		master_ = history_tree_ = new HistoryNode(nullptr, HistoryNode::Status::ACTIVE);

		auto leaf = new LeafNodeT(history_tree_->txn_id(), UUID::make_random());
		leaf->ref();
		history_tree_->set_root(leaf);

		SnapshotT snapshot(history_tree_, this);
		snapshot.freeze();
	}

	PersistentInMemAllocatorT(Int):
		metadata_(MetadataRepository<Profile>::getMetadata())
	{}

public:

	virtual ~PersistentInMemAllocatorT()
	{
		free_memory(history_tree_);
	}

	BigInt active_snapshots() const {
		return active_snapshots_;
	}

	struct shared : public MyType {};

	static auto create()
	{
		return std::make_shared<shared>();
	}

	auto newId()
	{
		return PageType::ID::make_random();
	}

	SnapshotPtr find(const TxnId& snapshot_id)
	{
		auto iter = snapshot_map_.find(snapshot_id);
		if (iter != snapshot_map_.end())
		{
			auto history_node = iter->second;

			if (history_node->is_committed())
			{
				return std::make_shared<SnapshotT>(history_node, this->shared_from_this());
			}
			else {
				throw vapi::Exception(MA_SRC, SBuf()<<"Snapshot "<<history_node->txn_id()<<" is "<<(history_node->is_active() ? "active" : "dropped"));
			}
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Snapshot id "<<snapshot_id<<" is not known");
		}
	}

	SnapshotPtr find_branch(StringRef name)
	{
		auto iter = named_branches_.find(name);
		if (iter != named_branches_.end())
		{
			auto history_node = iter->second;

			if (history_node->is_committed())
			{
				return std::make_shared<SnapshotT>(history_node, this->shared_from_this());
			}
			else {
				throw vapi::Exception(MA_SRC, SBuf()<<"Snapshot "<<history_node->txn_id()<<" is "<<(history_node->is_active() ? "active" : "dropped"));
			}
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Named branch \""<<name<<"\" is not known");
		}
	}

	SnapshotPtr master()
	{
		return std::make_shared<SnapshotT>(master_, this->shared_from_this());
	}

	void set_master(const TxnId& txn_id)
	{
		auto iter = snapshot_map_.find(txn_id);
		if (iter != snapshot_map_.end())
		{
			if (iter->second->is_committed())
			{
				master_ = iter->second;
			}
			else {
				throw vapi::Exception(MA_SRC, SBuf()<<"Snapshot "<<txn_id<<" hasn't been committed yet");
			}
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Snapshot "<<txn_id<<" is not known in this allocator");
		}
	}

	void set_branch(StringRef name, const TxnId& txn_id)
	{
		auto iter = snapshot_map_.find(txn_id);
		if (iter != snapshot_map_.end())
		{
			if (iter->second->is_committed())
			{
				named_branches_[name] = iter->second;
			}
			else {
				throw vapi::Exception(MA_SRC, SBuf()<<"Snapshot "<<txn_id<<" hasn't been committed yet");
			}
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Snapshot "<<txn_id<<" is not known in this allocator");
		}
	}

	ContainerMetadataRepository* getMetadata() const {
		return metadata_;
	}

	virtual void walkContainers(vapi::ContainerWalker* walker, const char* allocator_descr = nullptr)
	{
		walker->beginAllocator("PersistentInMemAllocator", allocator_descr);

		walk_version_tree(history_tree_, [&](auto* history_tree_node, auto* txn){
			txn->walkContainers(walker);
		});

		walker->endAllocator();
	}

	virtual void store(OutputStreamHandler *output)
	{
		if (active_snapshots_) {
			throw vapi::Exception(MA_SRC, "There are active snapshots");
		}

		records_ = 0;

		char signature[12] = "MEMORIA";
		for (size_t c = 7; c < sizeof(signature); c++) signature[c] = 0;

		output->write(&signature, 0, sizeof(signature));

		write_metadata(*output);

		walk_version_tree(history_tree_, [&](const HistoryNode* history_tree_node) {
			write_history_node(*output, history_tree_node);
		});

		Checksum checksum;
		checksum.records() = records_;

		write(*output, checksum);

		output->close();
	}


	static std::shared_ptr<MyType> load(InputStreamHandler *input)
	{
		MyType* allocator = new MyType(0);

		char signature[12];

		input->read(signature, sizeof(signature));

		if (!(
				signature[0] == 'M' &&
				signature[1] == 'E' &&
				signature[2] == 'M' &&
				signature[3] == 'O' &&
				signature[4] == 'R' &&
				signature[5] == 'I' &&
				signature[6] == 'A'))
		{
			throw vapi::Exception(MEMORIA_SOURCE, SBuf()<<"The stream does not start from MEMORIA signature: "<<signature);
		}

		if (!(signature[7] == 0 || signature[7] == 1))
		{
			throw vapi::BoundsException(MEMORIA_SOURCE, SBuf()<<"Endiannes filed value is out of bounds "<<signature[7]);
		}

		if (signature[8] != 0)
		{
			throw vapi::Exception(MEMORIA_SOURCE, "This is not an in-memory container");
		}

		allocator->master_ = allocator->history_tree_ = nullptr;

		allocator->snapshot_map_.clear();

		BigInt size = input->size();

		HistoryTreeNodeMap 		history_node_map;
		PersistentTreeNodeMap	ptree_node_map;
		PageMap					page_map;

		AllocatorMetadata metadata;

		Checksum checksum;

		while (input->pos() < size)
		{
			UByte type;
			*input >> type;

			switch (type)
			{
				case TYPE_METADATA: 	allocator->read_metadata(*input, metadata); break;
				case TYPE_DATA_PAGE: 	allocator->read_data_page(*input, page_map); break;
				case TYPE_LEAF_NODE: 	allocator->read_leaf_node(*input, ptree_node_map); break;
				case TYPE_BRANCH_NODE: 	allocator->read_branch_node(*input, ptree_node_map); break;
				case TYPE_HISTORY_NODE: allocator->read_history_node(*input, history_node_map); break;
				case TYPE_CHECKSUM: 	allocator->read_checksum(*input, checksum); break;
				default:
					throw vapi::Exception(MA_SRC, SBuf()<<"Unknown record type: "<<(Int)type);
			}

			allocator->records_++;
		}

		if (allocator->records_ != checksum.records())
		{
			throw vapi::Exception(MA_SRC, SBuf()<<"Invalid records checksum: actual="<<allocator->records_<<", expected="<<checksum.records());
		}

		for (auto& entry: ptree_node_map)
		{
			auto buffer = entry.second.first;
			auto node   = entry.second.second;

			if (buffer->is_leaf())
			{
				LeafNodeBufferT* leaf_buffer = static_cast<LeafNodeBufferT*>(buffer);
				LeafNodeT* leaf_node 		 = static_cast<LeafNodeT*>(node);

				for (Int c = 0; c < leaf_node->size(); c++)
				{
					const auto& descr = leaf_buffer->data(c);

					auto page_iter = page_map.find(descr.page());

					if (page_iter != page_map.end())
					{
						leaf_node->data(c) = typename LeafNodeT::Value(page_iter->second, descr.txn_id());
					}
					else {
						throw vapi::Exception(MA_SRC, SBuf()<<"Specified uuid "<<descr.page()<<" is not found in the page map");
					}
				}
			}
			else {
				BranchNodeBufferT* branch_buffer = static_cast<BranchNodeBufferT*>(buffer);
				BranchNodeT* branch_node 		 = static_cast<BranchNodeT*>(node);

				for (Int c = 0; c < branch_node->size(); c++)
				{
					const auto& node_id = branch_buffer->data(c);

					auto iter = ptree_node_map.find(node_id);

					if (iter != ptree_node_map.end())
					{
						branch_node->data(c) = iter->second.second;
					}
					else {
						throw vapi::Exception(MA_SRC, SBuf()<<"Specified uuid "<<node_id<<" is not found in the persistent tree node map");
					}
				}
			}
		}

		allocator->history_tree_ = allocator->build_history_tree(metadata.root(), nullptr, history_node_map, ptree_node_map);

		if (allocator->snapshot_map_.find(metadata.master()) != allocator->snapshot_map_.end())
		{
			allocator->master_ = allocator->snapshot_map_[metadata.master()];
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Specified master uuid "<<metadata.master()<<" is not found in the data");
		}

		for (auto& entry: metadata.named_branches())
		{
			auto iter = allocator->snapshot_map_.find(entry.second);

			if (iter != allocator->snapshot_map_.end())
			{
				allocator->named_branches_[entry.first] = iter->second;
			}
			else {
				throw vapi::Exception(MA_SRC, SBuf()<<"Specified snapshot uuid "<<entry.first<<" is not found");
			}
		}

		// Delete temporary buffers

		for (auto& entry: ptree_node_map)
		{
			auto buffer = entry.second.first;

			if (buffer->is_leaf())
			{
				delete static_cast<LeafNodeBufferT*>(buffer);
			}
			else {
				delete static_cast<BranchNodeBufferT*>(buffer);
			}
		}

		for (auto& entry: history_node_map)
		{
			auto node = entry.second;
			delete node;
		}

		return std::shared_ptr<MyType>(allocator);
	}

private:

	HistoryNode* build_history_tree(
		const TxnId& txn_id,
		HistoryNode* parent,
		const HistoryTreeNodeMap& history_map,
		const PersistentTreeNodeMap& ptree_map
	)
	{
		auto iter = history_map.find(txn_id);

		if (iter != history_map.end())
		{
			HistoryNodeBuffer* buffer = iter->second;

			HistoryNode* node = new HistoryNode(txn_id, parent, buffer->status());

			node->root_id() = buffer->root_id();
			node->metadata() = buffer->metadata();

			snapshot_map_[txn_id] = node;

			if (!buffer->root().is_null())
			{
				auto ptree_iter = ptree_map.find(buffer->root());

				if (ptree_iter != ptree_map.end())
				{
					node->set_root(ptree_iter->second.second);

					for (const auto& child_txn_id: buffer->children())
					{
						build_history_tree(child_txn_id, node, history_map, ptree_map);
					}

					return node;
				}
				else {
					throw vapi::Exception(MA_SRC, SBuf()<<"Specified node_id "<<buffer->root()<<" is not found in persistent tree data");
				}
			}
			else {
				for (const auto& child_txn_id: buffer->children())
				{
					build_history_tree(child_txn_id, node, history_map, ptree_map);
				}

				return node;
			}
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Specified txn_id "<<txn_id<<" is not found in history data");
		}
	}


	void free_memory(HistoryNode* node)
	{
		for (auto child: node->children())
		{
			free_memory(child);
		}

		if (node->root()) {
			free_memory(node->root());
		}

		delete node;
	}


	/**
	 * Deletes recursively all data quickly. Must not be used to delete data of a selected snapshot.
	 */

	void free_memory(const NodeBaseT* node)
	{
		const auto& txn_id = node->txn_id();
		if (node->is_leaf())
		{
			auto leaf = PersistentTreeT::to_leaf_node(node);

			for (Int c = 0; c < leaf->size(); c++)
			{
				auto& data = leaf->data(c);
				if (data.txn_id() == txn_id)
				{
					::free(data.page());
				}
			}

			delete leaf;
		}
		else {
			auto branch = PersistentTreeT::to_branch_node(node);

			for (Int c = 0; c < branch->size(); c++)
			{
				auto child = branch->data(c);
				if (child->txn_id() == txn_id)
				{
					free_memory(child);
				}
			}

			delete branch;
		}
	}

	void read_metadata(InputStreamHandler& in, AllocatorMetadata& metadata)
	{
		in >> metadata.master();
		in >> metadata.root();

		BigInt size;

		in >>size;

		for (BigInt c = 0; c < size; c++)
		{
			String name;
			in >> name;

			TxnId value;
			in >> value;

			metadata.named_branches()[name] = value;
		}
	}

	void read_checksum(InputStreamHandler& in, Checksum& checksum)
	{
		in >> checksum.records();
	}

	void read_data_page(InputStreamHandler& in, PageMap& map)
	{
		Int page_data_size;
		in >> page_data_size;

		Int page_size;
		in >> page_size;

		Int ctr_hash;
		in >> ctr_hash;

		Int page_hash;
		in >> page_hash;

		unique_ptr<Byte, void (*)(void*)> page_data((Byte*)::malloc(page_data_size), ::free);

		Page* page = T2T<Page*>(::malloc(page_size));

		in.read(page_data.get(), 0, page_data_size);

		PageMetadata* pageMetadata = metadata_->getPageMetadata(ctr_hash, page_hash);
		pageMetadata->getPageOperations()->deserialize(page_data.get(), page_data_size, T2T<void*>(page));

		if (map.find(page->uuid()) == map.end())
		{
			map[page->uuid()] = page;
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"Page "<<page->uuid()<<" was already registered");
		}
	}


	void read_leaf_node(InputStreamHandler& in, PersistentTreeNodeMap& map)
	{
		LeafNodeBufferT* buffer = new LeafNodeBufferT();

		buffer->read(in);

		if (map.find(buffer->node_id()) == map.end())
		{
			NodeBaseT* node = new LeafNodeT();
			node->populate_as_buffer(buffer);

			map[buffer->node_id()] = std::make_pair(buffer, node);
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"PersistentTree LeafNode "<<buffer->node_id()<<" was already registered");
		}
	}


	void read_branch_node(InputStreamHandler& in, PersistentTreeNodeMap& map)
	{
		BranchNodeBufferT* buffer = new BranchNodeBufferT();

		buffer->read(in);

		if (map.find(buffer->node_id()) == map.end())
		{
			NodeBaseT* node = new BranchNodeT();
			node->populate_as_buffer(buffer);

			map[buffer->node_id()] = std::make_pair(buffer, node);
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"PersistentTree BranchNode "<<buffer->node_id()<<" was already registered");
		}
	}




	void read_history_node(InputStreamHandler& in, HistoryTreeNodeMap& map)
	{
		HistoryNodeBuffer* node = new HistoryNodeBuffer();

		Int status;

		in >> status;

		node->status() = static_cast<typename HistoryNode::Status>(status);

		in >> node->txn_id();
		in >> node->root();
		in >> node->root_id();

		in >> node->parent();

		in >> node->metadata();

		BigInt children;
		in >>children;

		for (BigInt c = 0; c < children; c++)
		{
			TxnId child_txn_id;
			in >> child_txn_id;

			node->children().push_back(child_txn_id);
		}

		if (map.find(node->txn_id()) == map.end())
		{
			map[node->txn_id()] = node;
		}
		else {
			throw vapi::Exception(MA_SRC, SBuf()<<"HistoryTree Node "<<node->txn_id()<<" was already registered");
		}
	}







	std::unique_ptr<LeafNodeBufferT> to_leaf_buffer(const LeafNodeT* node)
	{
		std::unique_ptr<LeafNodeBufferT> buf = std::make_unique<LeafNodeBufferT>();

		buf->populate_as_buffer(node);

		for (Int c = 0; c < node->size(); c++)
		{
			buf->data(c) = typename LeafNodeBufferT::Value(
				node->data(c).page()->uuid(),
				node->data(c).txn_id()
			);
		}

		return buf;
	}

	std::unique_ptr<BranchNodeBufferT> to_branch_buffer(const BranchNodeT* node)
	{
		std::unique_ptr<BranchNodeBufferT> buf = std::make_unique<BranchNodeBufferT>();

		buf->populate_as_buffer(node);

		for (Int c = 0; c < node->size(); c++)
		{
			buf->data(c) = node->data(c)->node_id();
		}

		return buf;
	}

	void walk_version_tree(const HistoryNode* node, std::function<void (const HistoryNode*, SnapshotT*)> fn)
	{
		if (node->is_committed())
		{
			SnapshotT txn(history_tree_, this->shared_from_this());
			fn(node, &txn);
		}

		for (auto child: node->children())
		{
			walk_version_tree(child, fn);
		}

	}

	void walk_version_tree(const HistoryNode* node, std::function<void (const HistoryNode*)> fn)
	{
		fn(node);

		for (auto child: node->children())
		{
			walk_version_tree(child, fn);
		}
	}

	void write_metadata(OutputStreamHandler& out)
	{
		UByte type = TYPE_METADATA;
		out << type;

		out << master_->txn_id();
		out << history_tree_->txn_id();

		out << (BigInt) named_branches_.size();

		for (auto& entry: named_branches_)
		{
			out<<entry.first;
			out<<entry.second->txn_id();
		}

		records_++;
	}

	void write(OutputStreamHandler& out, const Checksum& checksum)
	{
		UByte type = TYPE_CHECKSUM;
		out << type;
		out << checksum.records() + 1;
	}

	void write_history_node(OutputStreamHandler& out, const HistoryNode* history_node)
	{
		UByte type = TYPE_HISTORY_NODE;
		out << type;
		out << (Int)history_node->status();
		out << history_node->txn_id();

		if (history_node->root())
		{
			out << history_node->root()->node_id();
		}
		else {
			out << PTreeNodeId();
		}

		out << history_node->root_id();

		if (history_node->parent())
		{
			out << history_node->parent()->txn_id();
		}
		else {
			out << TxnId();
		}

		out << history_node->metadata();

		out << (BigInt)history_node->children().size();

		for (auto child: history_node->children())
		{
			out << child->txn_id();
		}

		records_++;

		if (history_node->root())
		{
			write_persistent_tree(out, history_node->root());
		}
	}

	void write_persistent_tree(OutputStreamHandler& out, const NodeBaseT* node)
	{
		const auto& txn_id = node->txn_id();

		if (node->is_leaf())
		{
			auto leaf = PersistentTreeT::to_leaf_node(node);
			auto buf  = to_leaf_buffer(leaf);

			write(out, buf.get());

			for (Int c = 0; c < leaf->size(); c++)
			{
				const auto& data = leaf->data(c);

				if (data.txn_id() == txn_id || data.page()->references() == 1)
				{
					write(out, data.page());
				}
			}
		}
		else {
			auto branch = PersistentTreeT::to_branch_node(node);
			auto buf    = to_branch_buffer(branch);

			write(out, buf.get());

			for (Int c = 0; c < branch->size(); c++)
			{
				auto child = branch->data(c);

				if (child->txn_id() == txn_id || child->refs() == 1)
				{
					write_persistent_tree(out, child);
				}
			}
		}
	}


	void write(OutputStreamHandler& out, const BranchNodeBufferT* node)
	{
		UByte type = TYPE_BRANCH_NODE;
		out << type;
		node->write(out);

		records_++;
	}

	void write(OutputStreamHandler& out, const LeafNodeBufferT* node)
	{
		UByte type = TYPE_LEAF_NODE;
		out << type;
		node->write(out);

		records_++;
	}

	void write(OutputStreamHandler& out, const PageType* page)
	{
		UByte type = TYPE_DATA_PAGE;
		out << type;

        PageMetadata* pageMetadata = metadata_->getPageMetadata(page->ctr_type_hash(), page->page_type_hash());

        auto page_size = page->page_size();
        unique_ptr<Byte, void (*)(void*)> buffer((Byte*)::malloc(page_size), ::free);

        const IPageOperations* operations = pageMetadata->getPageOperations();

        Int total_data_size = operations->serialize(page, buffer.get());

        out << total_data_size;
        out << page->page_size();
        out << page->ctr_type_hash();
        out << page->page_type_hash();

        out.write(buffer.get(), 0, total_data_size);

        records_++;
	}


	void dump(const Page* page, std::ostream& out = std::cout)
	{
		TextPageDumper dumper(out);

		PageMetadata* meta = metadata_->getPageMetadata(page->ctr_type_hash(), page->page_type_hash());

		meta->getPageOperations()->generateDataEvents(page, DataEventsParams(), &dumper);
	}

	auto ref_active() {
		return ++active_snapshots_;
	}

	auto unref_active() {
		return --active_snapshots_;
	}

	auto forget_snapshot(HistoryNode* history_node)
	{
		snapshot_map_.erase(history_node->txn_id());

		history_node->remove_from_parent();

		delete history_node;
	}
};


template <typename Profile = DefaultProfile<>>
using PersistentInMemAllocator = class PersistentInMemAllocatorT<
		Profile,
		typename ContainerCollectionCfg<Profile>::Types::Page
>;



}



#endif
