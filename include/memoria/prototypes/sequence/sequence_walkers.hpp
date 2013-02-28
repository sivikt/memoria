
// Copyright Victor Smirnov 2012-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_SEQUENCE_WALKERS_HPP
#define _MEMORIA_PROTOTYPES_SEQUENCE_WALKERS_HPP

#include <memoria/prototypes/sequence/names.hpp>
#include <memoria/core/container/container.hpp>
#include <memoria/core/tools/idata.hpp>

#include <memoria/prototypes/btree/btree_walkers.hpp>

#include <functional>

namespace memoria    {
namespace sequence 	 {

using namespace std;
using namespace btree;

template <typename Types>
class FindWalkerBase {
protected:
	typedef typename Types::Key 												Key;
	typedef Iter<typename Types::IterTypes> 									Iterator;

	Key key_;
	Int key_num_;

	Key prefix_;
	Int idx_;

	WalkDirection direction_;

	Int start_;

public:
	FindWalkerBase(Key key, Int key_num):
		key_(key), key_num_(key_num), prefix_(0), idx_(0)
	{}

	WalkDirection& direction() {
		return direction_;
	}

	const Int& start() const {
		return start_;
	}

	Int& start() {
		return start_;
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;

		iter.cache().setup(prefix_, 0);
	}

	void empty(Iterator& iter)
	{
		iter.cache().setup(0, 0);
	}

	Int idx() const {
		return idx_;
	}
};


template <typename Types>
class FindLTWalker: public FindWalkerBase<Types> {

	typedef FindWalkerBase<Types> 		Base;
	typedef typename Base::Key 			Key;

public:
	FindLTWalker(Key key, Int key_num): Base(key, key_num)
	{}

	template <typename Node>
	void operator()(const Node* node)
	{
		const typename Node::Map& map = node->map();

		Base::idx_ = map.findLTS(Base::key_num_, Base::key_ - Base::prefix_, Base::prefix_);

		if (node->level() != 0 && Base::idx_ == map.size())
		{
			Base::prefix_ -= map.key(0, map.size() - 1);
			Base::idx_--;
		}
	}
};

template <typename Types>
class FindLEWalker: public FindWalkerBase<Types> {

	typedef FindWalkerBase<Types> 		Base;
	typedef typename Base::Key 			Key;

public:
	FindLEWalker(Key key, Int key_num): Base(key, key_num)
	{}

	template <typename Node>
	void operator()(const Node* node)
	{
		const typename Node::Map& map = node->map();

		Base::idx_ = map.findLES(Base::key_num_, Base::key_ - Base::prefix_, Base::prefix_);

		if (node->level() != 0 && Base::idx_ == map.size())
		{
			Base::prefix_ -= map.key(0, map.size() - 1);
			Base::idx_--;
		}
	}
};


template <typename Types>
class FindRangeWalkerBase {
protected:
	typedef Iter<typename Types::IterTypes> Iterator;
	typedef Ctr<typename Types::CtrTypes> 	Container;

	Int idx_;

	WalkDirection direction_;

	Int start_;

public:
	FindRangeWalkerBase(): idx_(0) {}

	WalkDirection& direction() {
		return direction_;
	}

	const Int& start() const {
		return start_;
	}

	Int& start() {
		return start_;
	}

	void empty(Iterator& iter)
	{
		iter.cache().setup(0, 0);
	}

	Int idx() const
	{
		return idx_;
	}
};



template <typename Types>
class FindEndWalker: public FindRangeWalkerBase<Types> {

	typedef FindRangeWalkerBase<Types> 		Base;
	typedef typename Base::Iterator 		Iterator;
	typedef typename Base::Container 		Container;

	typedef typename Types::Key 			Key;

	Key prefix_;

public:
	FindEndWalker(Container& ctr): prefix_(0) {}

	template <typename Node>
	void operator()(const Node* node)
	{
		const typename Node::Map& map = node->map();

		Base::idx_ = node->children_count() - 1;

		prefix_ += map.maxKey(0) - map.key(0, map.size() - 1);
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;

		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = iter.data()->size();

		iter.cache().setup(prefix_, 0);
	}
};


template <typename Types>
class FindREndWalker: public FindRangeWalkerBase<Types> {

	typedef FindRangeWalkerBase<Types> 		Base;
	typedef typename Base::Iterator 		Iterator;
	typedef typename Base::Container 		Container;

public:
	FindREndWalker(Container&) {}

	template <typename Node>
	void operator()(const Node* node)
	{
		Base::idx_ = 0;
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;

		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = -1;

		iter.cache().setup(0, 0);
	}
};




template <typename Types>
class FindBeginWalker: public FindRangeWalkerBase<Types> {

	typedef FindRangeWalkerBase<Types> 		Base;
	typedef typename Base::Iterator 		Iterator;
	typedef typename Base::Container 		Container;

public:
	FindBeginWalker(Container&) {}


	template <typename Node>
	void operator()(const Node* node)
	{
		Base::idx_ = 0;
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = 0;

		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = 0;

		iter.cache().setup(0, 0);
	}
};

template <typename Types>
class FindRBeginWalker: public FindRangeWalkerBase<Types> {

	typedef FindRangeWalkerBase<Types> 		Base;
	typedef typename Base::Iterator 		Iterator;
	typedef typename Base::Container 		Container;

	typedef typename Types::Key 			Key;

	Key prefix_;

public:
	FindRBeginWalker(Container&): prefix_(0) {}

	template <typename Node>
	void operator()(const Node* node)
	{
		const typename Node::Map& map = node->map();

		Base::idx_ = node->children_count() - 1;

		prefix_ += map.maxKey(0) - map.key(0, map.size() - 1);
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;

		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = iter.data()->size() - 1;

		iter.cache().setup(prefix_, 0);
	}
};


template <
	typename Types
>
class MultiPrefixWalkerBase {
protected:
	typedef typename Types::Key 												Key;
	typedef Iter<typename Types::IterTypes> 									Iterator;
	typedef Ctr<typename Types::CtrTypes> 										Container;

	Container& ctr_;

	Key key_;

	Int key_count_;
	const Int* key_nums_;

	Key* prefixes_;

	Int idx_;


	WalkDirection direction_;

	Int start_;

public:
	MultiPrefixWalkerBase(Container& ctr, Key key, Int key_count, const Int* key_nums, Key* prefixes):
		ctr_(ctr),
		key_(key),
		key_count_(key_count),
		key_nums_(key_nums),
		prefixes_(prefixes),
		idx_(0)
	{}

	WalkDirection& direction() {
		return direction_;
	}

	const Int& start() const {
		return start_;
	}

	Int& start() {
		return start_;
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;

		ctr_.finishPathStep(iter.path(), iter.key_idx());
	}

	void empty(Iterator& iter) {}

	const Int& idx() const {
		return idx_;
	}

	Int& idx() {
		return idx_;
	}
};





template <
	typename Types
>
class SinglePrefixWalkerBase {
protected:
	typedef typename Types::Key 												Key;
	typedef Iter<typename Types::IterTypes> 									Iterator;
	typedef Ctr<typename Types::CtrTypes> 										Container;

	Container& ctr_;

	Int key_num_;

	BigInt prefix_;

	Int idx_;

	WalkDirection direction_;

	Int start_;

public:
	SinglePrefixWalkerBase(Container& ctr, Int key_num):
		ctr_(ctr),
		key_num_(key_num),
		prefix_(0),
		idx_(0)
	{}

	WalkDirection& direction() {
		return direction_;
	}

	const Int& start() const {
		return start_;
	}

	Int& start() {
		return start_;
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;

		ctr_.finishPathStep(iter.path(), iter.key_idx());
	}

	void empty(Iterator& iter) {}

	const Int& idx() const {
		return idx_;
	}

	Int& idx() {
		return idx_;
	}

	BigInt prefix() const {
		return prefix_;
	}
};



struct NoOpSkipFWHandler {
	template <typename Node>
	void handleNode(const Node* node, Int start, Int end) {}

	template <typename Data>
	void handleData(const Data* data, Int start, Int end) {}
};



template <
	typename Types,
	typename PageHandler = NoOpSkipFWHandler
>
class SequenceSkipFWWalker: public SinglePrefixWalkerBase<Types> {
protected:
	typedef SinglePrefixWalkerBase<Types>										Base;
	typedef typename Types::Key 												Key;
	typedef typename Base::Iterator 											Iterator;
	typedef typename Base::Container 											Container;

	BigInt key_;

public:
	SequenceSkipFWWalker(Container& ctr, Int key_num, BigInt key):
		Base(ctr, key_num), key_(key)
	{}

	template <typename Node>
	void operator()(const Node* node)
	{
		typedef typename Node::Map Map;

		const Map& map = node->map();

		Base::idx_ = map.findFwLT(Base::key_num_, Base::start_, key_ - Base::prefix_, Base::prefix_);

		if (Base::idx_ == map.size() && Base::direction_ == WalkDirection::DOWN)
		{
			Base::prefix_ -= map.key(Base::key_num_, map.size() - 1);
			Base::idx_--;
		}
	}
};


template <
	typename Types
>
class SequenceSkipBWWalker: public SinglePrefixWalkerBase<Types> {
protected:
	typedef SinglePrefixWalkerBase<Types>										Base;
	typedef typename Types::Key 												Key;
	typedef typename Base::Iterator 											Iterator;
	typedef typename Base::Container 											Container;

	BigInt key_;

public:
	SequenceSkipBWWalker(Container& ctr, Int key_num, Int key):
		Base(ctr, key_num), key_(key)
	{}

	template <typename Node>
	void operator()(const Node* node)
	{
		typedef typename Node::Map Map;

		const Map& map = node->map();

		Base::idx_ = map.findBwLE(Base::key_num_, Base::start_, key_ - Base::prefix_, Base::prefix_);

		if (Base::idx_ == -1 && Base::direction_ == WalkDirection::DOWN)
		{
			Base::prefix_ -= map.key(Base::key_num_, 0);
			Base::idx_++;
		}
	}
};







template <
	typename 											Types,

	template <
		typename 						MyType,
		typename 						Map,
		template <
			typename,
			typename,
			typename
		> 								class Extender,
		typename 						ExtenderState
	> 													class NodeWalker,

	template <
		typename 						MyType,
		typename 						Sequence,
		template <
			typename,
			typename,
			typename
		> 								class Extender,
		typename 						ExtenderState
	> 													class SequenceWalker,

	template <typename, typename, typename> 			class NodeExtender,
	template <typename, typename, typename> 			class SequenceExtender,

	typename 											ExtenderState
>
class SequenceForwardWalker: public btree::BTreeForwardWalker<Types, NodeWalker, NodeExtender, ExtenderState> {
protected:

	typedef btree::BTreeForwardWalker<Types, NodeWalker, NodeExtender, ExtenderState> 					Base;

	typedef SequenceForwardWalker<
				Types, NodeWalker, SequenceWalker, NodeExtender, SequenceExtender, ExtenderState
	>																									MyType;

	typedef Iter<typename Types::IterTypes>										Iterator;
	typedef typename Types::DataPageG											DataPageG;
	typedef typename Types::DataPage											DataPage;
	typedef typename DataPage::Sequence											Sequence;


	Int sequence_block_num_;

	ExtenderState& data_state_;

	BigInt data_length_ = 0;

public:
	SequenceForwardWalker(
			BigInt limit,
			Int node_block_num,
			Int sequence_block_num,
			ExtenderState& node_state,
			ExtenderState& data_state
		):
		Base(limit, node_block_num, node_state),
		sequence_block_num_(sequence_block_num),
		data_state_(data_state)
	{}

	void setup(Iterator& iter) {}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;
		iter.model().finishPathStep(iter.path(), idx);
	}

	bool dispatchFirstData(Iterator& iter)
	{
		const Sequence& seq = iter.data()->sequence();

		Int pos = iter.dataPos();

		SequenceWalker<
			Sequence, MyType, SequenceExtender, ExtenderState
		>
		walker(*this, seq, Base::limit_, sequence_block_num_, data_state_);

		Int idx = seq.findFw(iter.dataPos(), walker);

		data_length_ +=  (idx < seq.size()) ? (idx - pos + 1) : (idx - pos);

		iter.dataPos() = idx;

		return idx < seq.size();
	}


	void dispatchLastData(Iterator& iter)
	{
		const Sequence& seq = iter.data()->sequence();

		SequenceWalker<
			Sequence, MyType, SequenceExtender, ExtenderState
		>
		walker(*this, seq, Base::limit_, sequence_block_num_, data_state_);

		Int idx = seq.findFw(0, walker);

		data_length_ +=  (idx < seq.size()) ? (idx + 1) : idx;

		iter.dataPos() = idx;
	}

	void finishEof(Iterator& iter)
	{
		iter.key_idx() = iter.page()->children_count() - 1;
		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = iter.data()->size();
	}

	BigInt data_length() const {
		return data_length_;
	}
};








template <
	typename 											Types,

	template <
		typename 						MyType,
		typename 						Map,
		template <
			typename,
			typename,
			typename
		> 								class Extender,
		typename 						ExtenderState
	> 													class NodeWalker,

	template <
		typename 						MyType,
		typename 						Sequence,
		template <
			typename,
			typename,
			typename
		> 								class Extender,
		typename 						ExtenderState
	> 													class SequenceWalker,

	template <typename, typename, typename> 			class NodeExtender,
	template <typename, typename, typename> 			class SequenceExtender,

	typename ExtenderState
>
class SequenceBackwardWalker: public btree::BTreeBackwardWalker<Types, NodeWalker, NodeExtender, ExtenderState> {
protected:

	typedef btree::BTreeBackwardWalker<Types, NodeWalker, NodeExtender, ExtenderState> 					Base;

	typedef SequenceBackwardWalker<
		Types, NodeWalker, SequenceWalker, NodeExtender, SequenceExtender, ExtenderState
	>																									MyType;

	typedef Iter<typename Types::IterTypes>										Iterator;
	typedef typename Types::DataPageG											DataPageG;
	typedef typename Types::DataPage											DataPage;
	typedef typename DataPage::Sequence											Sequence;


	Int sequence_block_num_;

	ExtenderState& data_state_;

	BigInt data_length_ = 0;

public:
	SequenceBackwardWalker(
			BigInt limit,
			Int node_block_num,
			Int sequence_block_num,
			ExtenderState& node_state,
			ExtenderState& data_state
	):
		Base(limit, node_block_num, node_state),
		sequence_block_num_(sequence_block_num),
		data_state_(data_state)
	{}

	void setup(Iterator& iter) {}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;
		iter.model().finishPathStep(iter.path(), idx);
	}

	bool dispatchFirstData(Iterator& iter)
	{
		const Sequence& seq = iter.data()->sequence();

		SequenceWalker<
			Sequence, MyType, SequenceExtender, ExtenderState
		>
		walker(*this, seq, Base::limit_, sequence_block_num_, data_state_);

		Int idx = seq.findBw(iter.dataPos(), walker);

		if (walker.is_found())
		{
			data_length_ += iter.dataPos() - idx;

			iter.dataPos() = idx;
		}
		else {
			data_length_ += iter.dataPos();
		}

		return walker.is_found();
	}

	void dispatchLastData(Iterator& iter)
	{
		const Sequence& seq = iter.data()->sequence();

		SequenceWalker<
			Sequence, MyType, SequenceExtender, ExtenderState
		>
		walker(*this, seq, Base::limit_, sequence_block_num_, data_state_);

		Int idx = seq.findBw(iter.data()->size() - 1, walker);

		if (walker.is_found())
		{
			iter.dataPos() = idx;

			data_length_ += iter.data()->size() - iter.dataPos();
		}
		else {
			iter.dataPos() = -1;

			data_length_ += iter.data()->size();
		}
	}

	void finishBof(Iterator& iter)
	{
		iter.key_idx() = 0;
		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = -1;
	}

	BigInt data_length() const
	{
		return data_length_;
	}
};







template <
	typename Types,

	template <typename Walker, typename Map, typename State> 		class NodeWalkerExtender,
	template <typename Walker, typename Sequence, typename State>	class SequenceWalkerExtender,

	typename ExtenderState
>
class SequenceSkipForwardWalker: public BTreeForwardWalker<
	Types, NodeLEForwardWalker, NodeWalkerExtender, ExtenderState
> {
protected:

	typedef btree::BTreeForwardWalker<Types, NodeLEForwardWalker, NodeWalkerExtender, ExtenderState> 		Base;

	typedef SequenceSkipForwardWalker<Types, NodeWalkerExtender, SequenceWalkerExtender, ExtenderState>		MyType;

	typedef Iter<typename Types::IterTypes>													Iterator;

	typedef typename Types::DataPage::Sequence												Sequence;

	BigInt prefix_ = 0;

	ExtenderState& data_state_;

public:
	SequenceSkipForwardWalker(BigInt limit, Int node_block_num, ExtenderState& node_state, ExtenderState& data_state):
		Base(limit, node_block_num, node_state),
		data_state_(data_state)
	{}

	void setup(Iterator& iter)
	{
		prefix_ = iter.prefix();
	}

	bool dispatchFirstData(Iterator& iter)
	{
		auto data 	= iter.data();
		Int pos 	= iter.dataPos();

		SequenceWalkerExtender<
			MyType, Sequence, ExtenderState
		>
		extender(*this, iter.data()->sequence(), data_state_);

		if (pos + Base::limit_ < data->size())
		{
			extender.processValues(0, -1, iter.dataPos(), iter.dataPos() + Base::limit_);

			iter.dataPos() 	+= Base::limit_;
			Base::sum_ 		+= Base::limit_;

			return true;
		}
		else {
			extender.processValues(0, -1, iter.dataPos(), data->size());

			BigInt remainder = data->size() - pos;

			Base::sum_ 		+= remainder;
			Base::limit_ 	-= remainder;

			prefix_ += pos;

			return false;
		}
	}

	void dispatchLastData(Iterator& iter)
	{
		auto data = iter.data();

		SequenceWalkerExtender<
			MyType, Sequence, ExtenderState
		>
		extender(*this, iter.data()->sequence(), data_state_);

		iter.cache().setup(prefix_ + Base::sum_, 0);

		if (Base::limit_ < data->size())
		{
			iter.dataPos() 	= Base::limit_;
			Base::sum_ 		+= Base::limit_;
		}
		else {
			Base::sum_ 		+= data->size();
			Base::limit_ 	-= data->size();

			iter.dataPos() = data->size();
		}

		extender.processValues(0, -1, 0, iter.dataPos());
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;
		iter.model().finishPathStep(iter.path(), idx);
	}

	void finishEof(Iterator& iter)
	{
		iter.key_idx() = iter.page()->children_count() - 1;
		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = iter.data()->size();

		iter.cache().setup(prefix_ + Base::sum_ - iter.data()->size(), 0);
	}
};







template <
	typename Types,

	template <typename Walker, typename Map, typename State> 		class NodeWalkerExtender,
	template <typename Walker, typename Sequenc, typename State>	class SequenceWalkerExtender,

	typename ExtenderState
>
class SequenceSkipBackwardWalker:public BTreeBackwardWalker<
	Types, NodeLEBackwardWalker, NodeWalkerExtender, ExtenderState
> {
protected:

	typedef btree::BTreeBackwardWalker<Types, NodeLEBackwardWalker, NodeWalkerExtender, ExtenderState> 		Base;

	typedef SequenceSkipBackwardWalker<Types, NodeWalkerExtender, SequenceWalkerExtender, ExtenderState>	MyType;

	typedef Iter<typename Types::IterTypes>													Iterator;
	typedef typename Types::DataPage::Sequence												Sequence;

	BigInt prefix_ = 0;

	ExtenderState& data_state_;

public:
	SequenceSkipBackwardWalker(BigInt limit, Int node_block_num, ExtenderState& node_state, ExtenderState& data_state):
		Base(limit, node_block_num, node_state),
		data_state_(data_state)
	{}

	void setup(Iterator& iter)
	{
		prefix_ = iter.prefix();
	}

	bool dispatchFirstData(Iterator& iter)
	{
		auto data 	= iter.data();
		Int pos 	= iter.dataPos();

		SequenceWalkerExtender<
			MyType, Sequence, ExtenderState
		>
		extender(*this, iter.data()->sequence(), data_state_);

		if (pos >= Base::limit_)
		{
			extender.processValues(0, -1, iter.dataPos() - Base::limit_ + 1, iter.dataPos() + 1);

			iter.dataPos() 	-= Base::limit_;
			Base::sum_ 		+= Base::limit_;

			return true;
		}
		else {
			extender.processValues(0, -1, 0, iter.dataPos() + 1);

			BigInt remainder = pos + 1;

			Base::sum_ 		+= remainder;
			Base::limit_ 	-= remainder;

			prefix_ += remainder;

			return false;
		}
	}

	void dispatchLastData(Iterator& iter)
	{
		SequenceWalkerExtender<
			MyType, Sequence, ExtenderState
		>
		extender(*this, iter.data()->sequence(), data_state_);

		auto data = iter.data();

		Int data_size = data->size();

		iter.cache().setup(prefix_ - Base::sum_ - data_size, 0);

		if (Base::limit_ < data_size)
		{
			iter.dataPos() 	= data_size - Base::limit_ - 1;
			Base::sum_ 		+= Base::limit_;
		}
		else {
			Base::sum_ 		+= data_size;
			Base::limit_ 	-= data_size;

			iter.dataPos() = -1;
		}

		extender.processValues(0, -1, iter.dataPos() + 1, data_size);
	}

	void finish(Int idx, Iterator& iter)
	{
		iter.key_idx() = idx;
		iter.model().finishPathStep(iter.path(), idx);
	}

	void finishBof(Iterator& iter)
	{
		iter.key_idx() = 0;
		iter.model().finishPathStep(iter.path(), iter.key_idx());
		iter.dataPos() = -1;

		iter.cache().setup(0, 0);
	}
};





template <
	typename Sequence,
	typename MainWalker,
	template <typename K1, typename K2> class Comparator,
	template <typename Walker, typename Map, typename State> class Extender,
	typename State
>
class DataForwardWalkerBase: public btree::NodeWalkerBase {

protected:
	typedef typename Sequence::IndexKey IndexKey;

	BigInt 			sum_				= 0;
	BigInt 			limit_;

	const IndexKey* indexes_;

	MainWalker& 	main_walker_;

	Extender<MainWalker, Sequence, State> extender_;

	Int block_num_;

public:
	DataForwardWalkerBase(MainWalker& main_walker, const Sequence& data, BigInt limit, Int block_num, State& state):
		limit_(limit),
		main_walker_(main_walker),
		extender_(main_walker, data, state),
		block_num_(block_num)
	{
		indexes_ 	= data.indexes(block_num);
	}

	Int walkIndex(Int start, Int end)
	{
		Comparator<BigInt, IndexKey> compare;

		for (Int c = start; c < end; c++)
		{
			IndexKey key = indexes_[c];

			if (compare(key, limit_))
			{
				sum_ 	+= key;
				limit_ 	-= key;
			}
			else {
				extender_.processIndexes(sum_, block_num_, start, c);
				return c;
			}
		}

		extender_.processIndexes(sum_, block_num_, start, end);
		return end;
	}

	void finish()
	{
		main_walker_.adjust(sum_);
	}

	BigInt sum() const {
		return sum_;
	}
};











template <
	typename Sequence,
	typename MainWalker,
	template <typename K1, typename K2> class Comparator,
	template <typename Walker, typename Map, typename State> class Extender,
	typename State
>
class DataBackwardWalkerBase: public btree::NodeWalkerBase {
protected:
	typedef typename Sequence::IndexKey IndexKey;

	BigInt 			sum_				= 0;
	BigInt 			limit_;

	const IndexKey* indexes_;

	MainWalker& 	main_walker_;

	Extender<MainWalker, Sequence, State> extender_;

	Int block_num_;

public:
	DataBackwardWalkerBase(MainWalker& main_walker, const Sequence& data, BigInt limit, Int block_num, State& state):
		limit_(limit),
		main_walker_(main_walker),
		extender_(main_walker, data, state),
		block_num_(block_num)
	{
		indexes_ 	= data.indexes(block_num);
	}

	Int walkIndex(Int start, Int end)
	{
		Comparator<BigInt, IndexKey> compare;

		for (Int c = start; c > end; c--)
		{
			IndexKey key = indexes_[c];

			if (compare(key, limit_))
			{
				sum_ 	+= key;
				limit_ 	-= key;
			}
			else {
				extender_.processIndexes(sum_, block_num_, c + 1, start + 1);
				return c;
			}
		}

		extender_.processIndexes(sum_, block_num_, end + 1, start + 1);

		return end;
	}

	void finish()
	{
		main_walker_.adjust(sum_);
	}

	BigInt sum() const {
		return sum_;
	}
};











}
}

#endif
