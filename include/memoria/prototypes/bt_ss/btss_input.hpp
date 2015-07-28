
// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_PROTOTYPES_BTSS_INPUT_HPP_
#define MEMORIA_PROTOTYPES_BTSS_INPUT_HPP_



#include <memoria/core/types/types.hpp>

#include <memoria/core/packed/tools/packed_dispatcher.hpp>

#include <memoria/prototypes/bt/layouts/bt_input.hpp>

namespace memoria 	{
namespace btss 		{


template <Int StreamIdx>
struct InputTupleSizeH {
	template <typename Tuple>
	static auto get(Tuple&& buffer, Int idx) -> Int {
		return 0;
	}

	template <typename Tuple, typename SizeT>
	static void add(Tuple&& buffer, Int idx, SizeT value)
	{}
};


template <Int StreamIdx>
struct LeafStreamSizeH {
	template <typename Stream, typename SizeT>
	static void stream(Stream* buffer, Int idx, SizeT value)
	{}
};









template <typename CtrT>
class AbstractBTSSInputProviderBase: public AbstractInputProvider<typename CtrT::Types> {
	using Base = AbstractInputProvider<typename CtrT::Types>;

public:
	using NodeBaseG = typename CtrT::Types::NodeBaseG;
	using CtrSizeT 	= typename CtrT::Types::CtrSizeT;

	using Position	= typename Base::Position;

	using InputTuple 		= typename CtrT::Types::template StreamInputTuple<0>;
	using InputTupleAdapter = typename CtrT::Types::template InputTupleAdapter<0>;

	using Buffer = std::vector<InputTuple>;

protected:
	Buffer buffer_;
	Int start_ = 0;
	Int size_ = 0;

	CtrT& 	ctr_;

	Int last_symbol_ = -1;

public:

	AbstractBTSSInputProviderBase(CtrT& ctr, Int capacity): Base(), buffer_(capacity), ctr_(ctr){}

	CtrT& ctr() {return ctr_;}
	const CtrT& ctr() const {return ctr_;}

	virtual bool hasData()
	{
		bool buffer_has_data = start_ < size_;

		return buffer_has_data || populate_buffer();
	}

	virtual Position fill(NodeBaseG& leaf, const Position& from)
	{
		Position pos = from;

		while(true)
		{
			auto buffer_sizes = this->buffer_size();

			if (buffer_sizes == 0)
			{
				if (!this->populate_buffer())
				{
					return pos;
				}
				else {
					buffer_sizes = this->buffer_size();
				}
			}

			auto capacity = this->findCapacity(leaf, buffer_sizes);

			if (capacity > 0)
			{
				insertBuffer(leaf, pos[0], capacity);

				auto rest = this->buffer_size();

				if (rest > 0)
				{
					return Position(pos[0] + buffer_sizes);
				}
				else {
					pos[0] += capacity;
				}
			}
			else {
				return pos;
			}
		}
	}

	virtual Int findCapacity(const NodeBaseG& leaf, Int size) = 0;

//	virtual Int findCapacity(const NodeBaseG& leaf, Int size)
//	{
//		if (checkSize(leaf, size))
//		{
//			return size;
//		}
//		else {
//			auto imax 			= size;
//			decltype(imax) imin = 0;
//			auto accepts 		= 0;
//
//			while (accepts < 50 && imax > imin)
//			{
//				if (imax - 1 != imin)
//				{
//					auto mid = imin + ((imax - imin) / 2);
//
//					if (this->checkSize(leaf, mid))
//					{
//						accepts++;
//						imin = mid + 1;
//					}
//					else {
//						imax = mid - 1;
//					}
//				}
//				else {
//					if (this->checkSize(leaf, imax))
//					{
//						accepts++;
//					}
//
//					break;
//				}
//			}
//
//			if (imax < size)
//			{
//				return imax;
//			}
//			else {
//				return size;
//			}
//		}
//	}

//	bool checkSize(const NodeBaseG& leaf, CtrSizeT target_size)
//	{
//		return ctr_.checkCapacities(leaf, Position(target_size));
//	}


//	virtual Int findCapacity(const NodeBaseG& leaf, Int size)
//	{
//		Int capacity = this->ctr_.getLeafNodeCapacity(leaf, 0);
//
//		if (capacity > size)
//		{
//			return size;
//		}
//		else {
//			return capacity;
//		}
//	}




	struct InsertBufferFn {

		template <Int StreamIdx, Int AllocatorIdx, Int Idx, typename StreamObj>
		void stream(StreamObj* stream, Int at, Int start, Int size, const Buffer& buffer)
		{
			stream->_insert(at, size, [&](Int idx){
				return std::get<Idx>(buffer[idx + start]);
			});
		}

		template <typename NodeTypes, typename... Args>
		void treeNode(LeafNode<NodeTypes>* leaf, Args&&... args)
		{
			leaf->processSubstreamGroups(*this, std::forward<Args>(args)...);
		}
	};


	virtual void insertBuffer(NodeBaseG& leaf, Int at, Int size)
	{
		CtrT::Types::Pages::LeafDispatcher::dispatch(leaf, InsertBufferFn(), at, start_, size, buffer_);
		start_ += size;
	}


	const Buffer& buffer() const {
		return buffer_;
	}

	Int buffer_size() const
	{
		return size_ - start_;
	}


	void dumpBuffer() const
	{
		cout<<"Level 0"<<", start: "<<start_<<" size: "<<size_<<" capacity: "<<buffer_.size()<<endl;
		for (Int c = start_; c < size_; c++)
		{
			cout<<c<<": "<<buffer_[c]<<endl;
		}
		cout<<endl;
	}

	// must be an abstract method
	virtual bool get(InputTuple& value) = 0;

	virtual bool populate_buffer()
	{
		Int cnt = 0;

		this->start_ = 0;
		this->size_ = 0;

		Int capacity = buffer_.size();

		while (true)
		{
			InputTuple value;

			if (get(buffer_[cnt]))
			{
				if (cnt++ == capacity)
				{
					break;
				}
			}
			else {
				break;
			}
		}

		this->size_ = cnt;

		return cnt > 0;
	}
};













template <
	typename CtrT,
	LeafDataLengthType LeafDataLength
>
class AbstractBTSSInputProvider;




template <typename CtrT>
class AbstractBTSSInputProvider<CtrT, LeafDataLengthType::FIXED>: public AbstractBTSSInputProviderBase<CtrT> {
	using Base = AbstractBTSSInputProviderBase<CtrT>;

public:
	using NodeBaseG = typename CtrT::Types::NodeBaseG;
	using CtrSizeT 	= typename CtrT::Types::CtrSizeT;

	using Position	= typename Base::Position;

	using InputTuple 		= typename Base::InputTuple;
	using InputTupleAdapter = typename Base::InputTupleAdapter;

	using Buffer = typename Base::Buffer;

public:

	AbstractBTSSInputProvider(CtrT& ctr, Int capacity): Base(ctr, capacity) {}


//	virtual Int findCapacity(const NodeBaseG& leaf, Int size)
//	{
//		if (checkSize(leaf, size))
//		{
//			return size;
//		}
//		else {
//			auto imax 			= size;
//			decltype(imax) imin = 0;
//			auto accepts 		= 0;
//
//			while (accepts < 50 && imax > imin)
//			{
//				if (imax - 1 != imin)
//				{
//					auto mid = imin + ((imax - imin) / 2);
//
//					if (this->checkSize(leaf, mid))
//					{
//						accepts++;
//						imin = mid + 1;
//					}
//					else {
//						imax = mid - 1;
//					}
//				}
//				else {
//					if (this->checkSize(leaf, imax))
//					{
//						accepts++;
//					}
//
//					break;
//				}
//			}
//
//			if (imax < size)
//			{
//				return imax;
//			}
//			else {
//				return size;
//			}
//		}
//	}


	virtual Int findCapacity(const NodeBaseG& leaf, Int size)
	{
		Int capacity = this->ctr_.getLeafNodeCapacity(leaf, 0);

		if (capacity > size)
		{
			return size;
		}
		else {
			return capacity;
		}
	}


//	bool checkSize(const NodeBaseG& leaf, CtrSizeT target_size)
//	{
//		return this->ctr_.checkCapacities(leaf, Position(target_size));
//	}
};



}
}






#endif