
// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_PROTOTYPES_BT_INPUT_HPP_
#define MEMORIA_PROTOTYPES_BT_INPUT_HPP_

#include <memoria/core/types/types.hpp>
#include <memoria/core/packed/tools/packed_dispatcher.hpp>

#include <memoria/core/packed/sseq/packed_fse_searchable_seq.hpp>

#include <memoria/core/tools/bitmap.hpp>
#include <memoria/core/packed/tools/packed_malloc.hpp>
#include <memoria/core/exceptions/memoria.hpp>

#include <cstdlib>
#include <tuple>

namespace memoria 	{
namespace bt 		{


template <typename Types>
struct AbstractInputProvider {
	using Position = typename Types::Position;
	using NodeBaseG = typename Types::NodeBaseG;

	virtual bool hasData() = 0;
	virtual Position fill(NodeBaseG& leaf, const Position& from)	= 0;
};




namespace detail {

template <typename Types, Int Streams, Int Idx = 0>
struct InputBufferBuilder {
	using InputTuple 	= typename Types::template StreamInputTuple<Idx>;
	using SizeT 		= typename Types::CtrSizeT;

	using Type = MergeLists<
			std::vector<InputTuple>,
			typename InputBufferBuilder<Types, Streams, Idx + 1>::Type
	>;
};

template <typename Types, Int Streams>
struct InputBufferBuilder<Types, Streams, Streams> {
	using Type = TL<>;
};








template <typename List>  struct AsTuple;

template <typename... Types>
struct AsTuple<TL<Types...>> {
	using Type = std::tuple<Types...>;
};


template <Int Size, Int Idx = 0>
struct ForAllTuple {
	template <typename InputBuffer, typename Fn, typename... Args>
	static void process(InputBuffer&& tuple, Fn&& fn, Args&&... args)
	{
		fn.template process<Idx>(std::get<Idx>(tuple), std::forward<Args>(args)...);
		ForAllTuple<Size, Idx + 1>::process(tuple, std::forward<Fn>(fn), std::forward<Args>(args)...);
	}
};

template <Int Idx>
struct ForAllTuple<Idx, Idx> {
	template <typename InputBuffer, typename Fn, typename... Args>
	static void process(InputBuffer&& tuple, Fn&& fn, Args&&... args)
	{}
};



template <typename InputBuffer, Int Idx = 0, Int Size = std::tuple_size<InputBuffer>::value - 1> struct RankHelper;
template <typename InputBuffer, Int Idx, Int Size>
struct RankHelper {
	template <typename Fn, typename... Args>
	static void process(const InputBuffer& tuple, Fn&& fn, Args&&... args)
	{
		using NextHelper = RankHelper<InputBuffer, Idx + 1, Size>;
		fn.template process<Idx, NextHelper>(tuple, std::forward<Args>(args)...);
	}
};


template <typename InputBuffer, Int Idx>
struct RankHelper<InputBuffer, Idx, Idx> {
	template <typename Fn, typename... Args>
	static void process(const InputBuffer& tuple, Fn&& fn, Args&&... args)
	{
		fn.template processLast<Idx>(tuple, std::forward<Args>(args)...);
	}
};



template <
	template <Int> class TupleHelper,
	typename CtrSizeT,
	typename Position
>
struct RankFn {
	CtrSizeT pos_;
	CtrSizeT target_;

	Position start_;
	Position size_;
	Position prefix_;
	Position indexes_;

	RankFn(const Position& start, const Position& size, const Position& prefix, CtrSizeT target):
		target_(target),
		start_(start),
		size_(size),
		prefix_(prefix),
		indexes_(prefix)
	{
		pos_ = prefix.sum();
	}

	template <Int Idx, typename NextHelper, typename Buffer>
	void process(Buffer&& buffer, CtrSizeT length)
	{
		for (auto i = 0; i < length; i++)
		{
			if (pos_ < target_)
			{
				auto next_length = TupleHelper<Idx>::get(std::get<Idx>(buffer), i + prefix_[Idx] + start_[Idx]);

				indexes_[Idx]++;
				pos_++;

				NextHelper::process(buffer, *this, next_length);
			}
			else {
				break;
			}
		}
	}


	template <Int Idx, typename Buffer>
	void processLast(Buffer&& buffer, CtrSizeT length)
	{
		CtrSizeT size = size_[Idx] - start_[Idx];

		if (indexes_[Idx] + length > size)
		{
			length = size - indexes_[Idx];
		}

		if (pos_ + length < target_)
		{
			indexes_[Idx] += length;
			pos_ += length;
		}
		else
		{
			auto limit = target_ - pos_;
			indexes_[Idx] += limit;
			pos_ += limit;
		}
	}
};


template <
	Int Idx,
	Int Size
>
struct ZeroRankHelper
{
	template <typename Provider, typename Position1, typename Position2, typename... Args>
	static auto process(const Provider* provider, Position1&& sizes, Position2&& prefix, Args&&... args)
	{
		auto size = sizes[Idx];
		if (size > 0)
		{
			return provider->template _rank<Idx>(prefix, size, std::forward<Args>(args)...);
		}
		else {
			return ZeroRankHelper<Idx + 1, Size>::process(provider, std::forward<Position1>(sizes), std::forward<Position2>(prefix), std::forward<Args>(args)...);
		}
	}
};


template <Int Idx>
struct ZeroRankHelper<Idx, Idx> {
	template <typename Provider, typename Position1, typename Position2, typename... Args>
	static auto process(const Provider*, Position1&&, Position2&&, Args&&...)
	{
		return typename std::remove_reference<Position1>::type();
	}
};



template <
	template <Int> class SizeAccessor,
	Int Idx,
	Int Size
>
struct InputTupleSizeHelper {
	template <typename Buffer, typename... Args>
	static auto get(Int stream, Buffer&& buffer, Args&&... args) ->
		decltype(SizeAccessor<Idx>::get(std::get<Idx>(buffer), std::forward<Args>(args)...))
	{
		if (stream == Idx)
		{
			return SizeAccessor<Idx>::get(std::get<Idx>(buffer), std::forward<Args>(args)...);
		}
		else {
			return InputTupleSizeHelper<SizeAccessor, Idx + 1, Size>::get(stream, std::forward<Buffer>(buffer), std::forward<Args>(args)...);
		}
	}

	template <typename Buffer, typename... Args>
	static void add(Int stream, Buffer&& buffer, Args&&... args)
	{
		if (stream == Idx)
		{
			SizeAccessor<Idx>::add(std::get<Idx>(buffer), std::forward<Args>(args)...);
		}
		else {
			InputTupleSizeHelper<SizeAccessor, Idx + 1, Size>::add(stream, std::forward<Buffer>(buffer), std::forward<Args>(args)...);
		}
	}
};



template <
	template <Int> class SizeAccessor,
	Int Size
>
struct InputTupleSizeHelper<SizeAccessor, Size, Size> {
	template <typename Buffer, typename... Args>
	static Int get(Int stream, Buffer&& buffer, Args&&... args){
		return 0;
	}

	template <typename Buffer, typename... Args>
	static void add(Int stream, Buffer&& buffer, Args&&... args){}
};


template <
	typename Dispatcher,
	template<Int> class Fn,
	typename PathList,
	Int Idx = 0
>
struct UpdateLeafH;

template <
	typename Dispatcher,
	template<Int> class Fn,
	typename Path,
	typename... Tail,
	Int Idx
>
struct UpdateLeafH<Dispatcher, Fn, TL<Path, Tail...>, Idx> {

	template <typename NTypes, typename... Args>
	void treeNode(LeafNode<NTypes>* node, Args&&... args)
	{
		node->template processStream<Path>(Fn<Idx>(), std::forward<Args>(args)...);
	}

	template <typename PageG, typename... Args>
	void process(Int stream, PageG&& page, Args&&... args)
	{
		if (stream == Idx)
		{
			Dispatcher::dispatch(std::forward<PageG>(page), *this, std::forward<Args>(args)...);
		}
		else {
			UpdateLeafH<Dispatcher, Fn, TL<Tail...>, Idx + 1>::process(stream, std::forward<PageG>(page), std::forward<Args>(args)...);
		}
	}
};



template <
	typename Dispatcher,
	template<Int> class Fn,
	Int Idx
>
struct UpdateLeafH<Dispatcher, Fn, TL<>, Idx> {
	template <typename PageG, typename... Args>
	static void process(Int stream, PageG&& page, Args&&... args)
	{
		throw vapi::Exception(MA_SRC, SBuf()<<"Failed to dispatch stream: "<<stream);
	}
};

}








template <
	typename CtrT,
	Int Streams,
	LeafDataLengthType LeafDataLength
>
class AbstractCtrInputProvider;



template <
	typename CtrT,
	Int Streams
>
class AbstractCtrInputProvider<CtrT, Streams, LeafDataLengthType::FIXED>: public AbstractInputProvider<typename CtrT::Types> {

	using Base = AbstractInputProvider<typename CtrT::Types>;

public:
	using MyType = AbstractCtrInputProvider<CtrT, Streams, LeafDataLengthType::FIXED>;

	using NodeBaseG = typename CtrT::Types::NodeBaseG;
	using CtrSizeT 	= typename CtrT::Types::CtrSizeT;

	using Buffer = typename detail::AsTuple<
			typename detail::InputBufferBuilder<
				typename CtrT::Types,
				Streams
			>::Type
	>::Type;

	using Position	= typename Base::Position;

	template <Int, Int> friend struct detail::ZeroRankHelper;
	template <template <Int> class T, typename, typename> friend struct detail::RankFn;

	template <Int StreamIdx>
	using InputTupleSizeAccessor = typename CtrT::Types::template InputTupleSizeAccessor<StreamIdx>;

	using InputTupleSizeHelper = detail::InputTupleSizeHelper<InputTupleSizeAccessor, 0, std::tuple_size<Buffer>::value>;

	using ForAllBuffer = detail::ForAllTuple<std::tuple_size<Buffer>::value>;

	static constexpr Int get_symbols_number(Int v) {
		return v == 1 ?  0 : (v == 2 ? 1 : (v == 3 ? 2 : (v == 4 ? 2 : (v == 5 ? 3 : (v == 6 ? 3 : (v == 7 ? 3 : 4))))));
	}

	using Symbols = PkdFSSeq<typename PkdFSSeqTF<get_symbols_number(Streams)>::Type>;

	class RunDescr {
		Int symbol_;
		Int length_;
	public:
		RunDescr(Int symbol, Int length = 1): symbol_(symbol), length_(length) {}

		Int symbol() const {return symbol_;}
		Int length() const {return length_;}
	};

protected:
	Buffer buffer_;
	Position start_;
	Position size_;

	Position capacity_;

	FreeUniquePtr<PackedAllocator> allocator_;

	CtrT& 	ctr_;

	Position ancors_;
	NodeBaseG leafs_[Streams];

	Int last_symbol_ = -1;

private:



	struct ResizeBufferFn {
		template <Int Idx, typename Buffer>
		void process(Buffer&& buffer, const Position& sizes)
		{
			buffer.resize(sizes[Idx]);
		}
	};

	Symbols* symbols() {
		return allocator_->get<Symbols>(0);
	}

	const Symbols* symbols() const {
		return allocator_->get<Symbols>(0);
	}

public:

	AbstractCtrInputProvider(CtrT& ctr, const Position& capacity): Base(),
		capacity_(capacity),
		allocator_(AllocTool<PackedAllocator>::create(block_size(capacity.sum()), 1)),
		ctr_(ctr)
	{
		ForAllBuffer::process(buffer_, ResizeBufferFn(), capacity);

		allocator_->template allocate<Symbols>(0, Symbols::packed_block_size(capacity.sum()));

		symbols()->enlargeData(capacity.sum());
	}
private:
	static Int block_size(Int capacity)
	{
		return Symbols::packed_block_size(capacity);
	}
public:


	CtrT& ctr() {return ctr_;}
	const CtrT& ctr() const {return ctr_;}

	virtual bool hasData()
	{
		bool buffer_has_data = start_.sum() < size_.sum();

		return buffer_has_data || populate_buffer();
	}

	virtual Position fill(NodeBaseG& leaf, const Position& from)
	{
		Position pos = from;

		while(true)
		{
			auto buffer_sizes = this->buffer_size();

			if (buffer_sizes.sum() == 0)
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

			if (capacity.sum() > 0)
			{
				insertBuffer(leaf, pos, capacity);

				auto rest = this->buffer_size();

				if (rest.sum() > 0)
				{
					return pos + buffer_sizes;
				}
				else {
					pos += capacity;
				}
			}
			else {
				return pos;
			}
		}
	}

	virtual Position findCapacity(const NodeBaseG& leaf, const Position& sizes)
	{
		auto size  = sizes.sum();

		if (checkSize(leaf, size))
		{
			return sizes;
		}
		else {
			auto imax 			= size;
			decltype(imax) imin = 0;
			auto accepts 		= 0;

			Int last = imin;

			while (imax > imin)
			{
				if (imax - 1 != imin)
				{
					auto mid = imin + ((imax - imin) / 2);

					if (this->checkSize(leaf, mid))
					{
						if (accepts++ >= 50)
						{
							return rank(mid);
						}
						else {
							imin = mid + 1;
						}

						last = mid;
					}
					else {
						imax = mid - 1;
					}
				}
				else {
					if (this->checkSize(leaf, imax))
					{
						return rank(imax);
					}
					else {
						return rank(last);
					}
				}
			}

			return rank(last);
		}
	}



	bool checkSize(const NodeBaseG& leaf, CtrSizeT target_size)
	{
		auto rank = this->rank(target_size);
		return ctr_.checkCapacities(leaf, rank);
	}

	bool checkSize(const NodeBaseG& leaf, Position target_size)
	{
		return ctr_.checkCapacities(leaf, target_size);
	}


	struct InsertBufferFn {

		template <Int StreamIdx, Int AllocatorIdx, Int Idx, typename StreamObj>
		void stream(StreamObj* stream, const Position& at, const Position& starts, const Position& sizes, const Buffer& buffer)
		{
			static_assert(StreamIdx < Position::Indexes, "");
			static_assert(StreamIdx < tuple_size<Buffer>::value, "");
			static_assert(Idx < tuple_size<typename tuple_element<StreamIdx, Buffer>::type::value_type>::value, "");

			stream->_insert(at[StreamIdx], sizes[StreamIdx], [&](Int idx){
				return std::get<Idx>(std::get<StreamIdx>(buffer)[idx + starts[StreamIdx]]);
			});
		}


		template <typename NodeTypes, typename... Args>
		void treeNode(LeafNode<NodeTypes>* leaf, Args&&... args)
		{
			leaf->processSubstreamGroups(*this, std::forward<Args>(args)...);
		}
	};


	virtual void insertBuffer(NodeBaseG& leaf, const Position& at, const Position& sizes)
	{
		CtrT::Types::Pages::LeafDispatcher::dispatch(leaf, InsertBufferFn(), at, start_, sizes, buffer_);

		for (Int c = 0; c < Streams - 1; c++)
		{
			if (sizes[c] > 0)
			{
				ancors_[c] = at[c] + sizes[c] - 1;
				leafs_[c]  = leaf;
			}
		}

		start_ += sizes;
	}

	const Buffer& buffer() const {
		return buffer_;
	}

	Position buffer_size() const
	{
		return size_ - start_;
	}

	struct BufferCapacityFn {
		template <Int Idx, typename Buffer>
		void process(Buffer&& buffer, Position& pos)
		{
			pos[Idx] = buffer.size();
		}
	};

	Position capacity() const
	{
		return capacity_;
	}

	Position buffer_capacity() const
	{
		Position sizes;
		ForAllBuffer::process(buffer_, BufferCapacityFn(), sizes);
		return sizes;
	}


	Position rank(CtrSizeT idx) const
	{
		Position rnk;

		Int start_pos = start_.sum();

		auto symbols = this->symbols();

		for (Int s = 0; s < Streams; s++)
		{
			rnk[s] = symbols->rank(start_pos, idx + start_pos, s);
		}

		return rnk;
	}

	Position rank() const {
		return buffer_size();
	}


	virtual bool populate_buffer()
	{
		Position sizes;
		Position buffer_sums;

		start_.clear();
		size_.clear();

		Int symbol_pos = 0;

		auto capacity = this->buffer_capacity();

		auto symbols = this->symbols();

		symbols->size() = 0;

		auto syms = symbols->symbols();

		while (true)
		{
			RunDescr run = populate(size_);

			Int symbol = run.symbol();
			Int length = run.length();

			if (symbol >= 0)
			{
				symbols->size() += length;
				for (Int c = symbol_pos; c < symbol_pos + length; c++)
				{
					symbols->tools().set(syms, c, symbol);
				}

				symbol_pos += length;

				size_[symbol] += length;

				if (symbol > last_symbol_ + 1)
				{
					throw Exception(MA_SRC, SBuf()<<"Invalid sequence state: last_symbol="<<last_symbol_<<", symbol="<<symbol);
				}
				else if (symbol < last_symbol_)
				{
					this->finish_stream_run(symbol, last_symbol_, sizes, buffer_sums);
				}

				buffer_sums[symbol] += length;
				sizes[symbol] += length;

				if (size_[symbol] == capacity[symbol])
				{
					this->finish_stream_run(0, Streams - 1, sizes, buffer_sums);
					break;
				}

				last_symbol_ = symbol;
			}
			else {
				this->finish_stream_run(0, Streams - 1, sizes, buffer_sums);
				break;
			}
		}

		symbols->reindex();

		return symbol_pos > 0;
	}

	virtual RunDescr populate(const Position& pos) = 0;

	struct DumpFn {
		template <Int Idx, typename Buffer>
		void process(Buffer&& buffer, const Position& prefix, const Position& start, const Position& size)
		{
			cout<<"Level "<<Idx<<" prefix: "<<prefix[Idx]<<", start: "<<start[Idx]<<" size: "<<size[Idx]<<" capacity: "<<buffer.size()<<endl;
			for (Int c = start[Idx]; c < size[Idx]; c++)
			{
				cout<<c<<": "<<buffer[c]<<endl;
			}
			cout<<endl;
		}
	};


	void dumpBuffer() const
	{
		ForAllBuffer::process(buffer_, DumpFn(), Position(), start_, size_);

		symbols()->dump();
	}

private:

	void finish_stream_run(Int symbol, Int last_symbol, const Position& sizes, Position& buffer_sums)
	{
		for (Int sym = last_symbol; sym > symbol; sym--)
		{
			if (sizes[sym - 1] > 0)
			{
				if (buffer_sums[sym] > 0) {
					InputTupleSizeHelper::add(sym - 1, buffer_, sizes[sym - 1] - 1, buffer_sums[sym]);
				}
			}
			else if (leafs_[sym - 1].isSet())
			{
				if (buffer_sums[sym] > 0) {
					updateLeaf(sym - 1, ancors_[sym - 1], buffer_sums[sym]);
				}
			}

			buffer_sums[sym] = 0;
		}
	}

	void updateLeaf(Int sym, CtrSizeT pos, CtrSizeT sum)
	{
		detail::UpdateLeafH<
			typename CtrT::Types::Pages::LeafDispatcher,
			CtrT::Types::template LeafStreamSizeAccessor,
			typename CtrT::Types::StreamsSizes
		>().process(sym, leafs_[sym], pos, sum);
	}
};


}
}

#endif
