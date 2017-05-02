
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

#include <memoria/v1/core/types/types.hpp>
#include <memoria/v1/core/types/algo/for_each.hpp>

#include <memoria/v1/prototypes/bt_fl/btfl_names.hpp>
#include <memoria/v1/core/container/iterator.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include <iostream>

namespace memoria {
namespace v1 {


MEMORIA_V1_ITERATOR_PART_BEGIN(v1::btfl::IteratorSkipName)

    using typename Base::CtrSizeT;
    using typename Base::CtrSizesT;
    using Container = typename Base::Container;

    using typename Base::LeafDispatcher;

    template <typename LeafPath>
    using AccumItemH = typename Container::Types::template AccumItemH<LeafPath>;

    static const int32_t Streams                = Container::Types::Streams;
    static const int32_t DataStreams            = Container::Types::DataStreams;
    static const int32_t StructureStreamIdx     = Container::Types::StructureStreamIdx;

    using DataSizesT = typename Container::Types::DataSizesT;

public:
    bool isBegin() const
    {
        auto& self = this->self();
        return self.idx() < 0 || self.isEmpty();
    }

    bool isEnd() const
    {
        auto& self = this->self();

        return self.leaf().isSet() ? self.idx() >= self.leaf_size(StructureStreamIdx) : true;
    }

    bool is_end() const
    {
        auto& self = this->self();
        return self.leaf().isSet() ? self.idx() >= self.leaf_size(StructureStreamIdx) : true;
    }

    bool isEnd(int32_t idx) const
    {
        auto& self = this->self();

        return self.leaf().isSet() ? idx >= self.leaf_size(StructureStreamIdx) : true;
    }

    bool isContent() const
    {
        auto& self = this->self();
        return !(self.isBegin() || self.isEnd());
    }

    bool isContent(int32_t idx) const
    {
        auto& self = this->self();

        bool is_set = self.leaf().isSet();

        auto leaf_size = self.leaf_size(StructureStreamIdx);

        return is_set && idx >= 0 && idx < leaf_size;
    }

    bool isNotEnd() const
    {
        return !self().isEnd();
    }

    bool isEmpty() const
    {
        auto& self = this->self();
        return self.leaf().isEmpty() || self.leaf_size(StructureStreamIdx) == 0;
    }

    bool isNotEmpty() const
    {
        return !self().isEmpty();
    }

    void dumpKeys(std::ostream& out) const
    {
        auto& self = this->self();

        out<<"Stream:  "<<self.data_stream_s()<<std::endl;
        out<<"Idx:  "<<self.idx()<<std::endl;
    }





    bool next() {
        return self().skipFw(1) > 0;
    }

    bool prev() {
        return self().skipBw(1) > 0;
    }


    struct SkipFWFn {
    	template <int32_t StreamIdx, typename IterT>
    	auto process(IterT&& iter, CtrSizeT n)
    	{
    		return iter.template skip_fw_<StreamIdx>(n);
    	}
    };


    CtrSizeT skipFw(CtrSizeT n)
    {
    	auto& self = this->self();
    	int32_t stream = self.stream();
    	return ForEachStream<Streams - 1>::process(stream, SkipFWFn(), self, n);
    }

    struct SkipBWFn {
    	template <int32_t StreamIdx, typename IterT>
    	auto process(IterT&& iter, CtrSizeT n)
    	{
    		return iter.template skip_bw_<StreamIdx>(n);
    	}
    };

    CtrSizeT skipBw(CtrSizeT n)
    {
    	auto& self = this->self();
    	int32_t stream = self.stream();
    	return ForEachStream<Streams - 1>::process(stream, SkipBWFn(), self, n);
    }

    CtrSizeT skip(CtrSizeT n)
    {
        if (n > 0) {
            return skipFw(n);
        }
        else {
            return skipBw(n);
        }
    }

    int32_t data_stream() const
    {
        auto& self  = this->self();
        auto s      = self.leaf_structure();

        if (s != nullptr)
        {
        	auto idx    = self.idx();

        	if (idx < s->size())
        	{
        		return s->get_symbol(idx);
        	}
        }

        return -1;//throw Exception(MA_SRC, "End Of Data Structure");
    }

    int32_t data_stream_s() const
    {
        auto& self  = this->self();
        auto s      = self.leaf_structure();

        if (s != nullptr)
        {
        	auto idx    = self.idx();

        	if (idx < s->size())
        	{
        		return s->get_symbol(idx);
        	}
        }

        return -1;
    }









    CtrSizeT pos() const
    {
        auto& self = this->self();
        return self.template stream_size_prefix<StructureStreamIdx>() + self.idx();
    }

//    CtrSizeT stream_pos() const
//    {
//
//        return fn.pos_ + self.idx();
//    }


private:

    template <int32_t Stream>
    struct SizePrefix {
        CtrSizeT pos_ = 0;

        template <typename NodeTypes>
        void treeNode(const bt::BranchNode<NodeTypes>* node, WalkCmd cmd, int32_t start, int32_t end)
        {
            using BranchSizePath = IntList<Stream>;

            auto sizes_substream = node->template substream<BranchSizePath>();

            pos_ += sizes_substream->sum(0, end);
        }

        template <typename NodeTypes>
        void treeNode(const bt::LeafNode<NodeTypes>* node, WalkCmd cmd, int32_t start, int32_t end)
        {
        }
    };

public:
    template <int32_t Stream>
    auto stream_size_prefix() const
    {
        auto& self = this->self();
        SizePrefix<Stream> fn;

        self.ctr().walkUp(self.leaf(), self.idx(), fn);

        return fn.pos_;
    }

protected:

    void toStructureStream()
    {
    	auto& self = this->self();

    	if (self.stream() != StructureStreamIdx)
    	{
    		int32_t stream = self.stream();

    		int32_t data_idx = self.idx();

    		auto s = self.leaf_structure();

    		if (s != nullptr)
    		{
    			auto result = s->selectFW(data_idx + 1, stream);

    			self.stream() = StructureStreamIdx;

    			if (result.is_found())
    			{
    				self.idx() = result.idx();
    			}
    			else {
    				self.idx() = self.leaf_size(StructureStreamIdx);
    			}
    		}
    		else {
    			throw Exception(MA_SRC, SBuf() << "Structure stream is empty");
    		}
    	}
    	else {
    		throw Exception(MA_SRC, SBuf() << "Invalid stream: " << self.stream());
    	}
    }

    void toDataStream(int32_t stream)
    {
    	auto& self = this->self();

    	if (self.stream() == StructureStreamIdx)
    	{
    		int32_t data_idx 	= self.data_stream_idx(stream);
    		self.idx() 		= data_idx;
    		self.stream() 	= stream;
    	}
    	else {
    		throw Exception(MA_SRC, SBuf() << "Invalid stream: " << self.stream());
    	}
    }

    int32_t data_stream_idx(int32_t stream) const
    {
        auto& self = this->self();
        return self.data_stream_idx(stream, self.idx());
    }

    int32_t data_stream_idx(int32_t stream, int32_t structure_idx) const
    {
        auto& self = this->self();

        auto s = self.leaf_structure();
        if (s != nullptr) {
        	return s->rank(structure_idx, stream);
        }
        else {
        	return 0;
        }
    }

    CtrSizesT leafrank() const {
    	return self().leafrank(self().idx());
    }


    CtrSizesT leafrank(int32_t structure_idx) const
    {
        auto& self = this->self();

        auto leaf_structure = self.leaf_structure();

        if (leaf_structure != nullptr) {

        	CtrSizesT ranks;

        	for (int32_t c = 0; c < DataStreams; c++)
        	{
        		ranks[c] = leaf_structure->rank(structure_idx, c);
        	}

        	ranks[StructureStreamIdx] = structure_idx;

        	return ranks;
        }
        else {
        	CtrSizesT ranks;
        	ranks[StructureStreamIdx] = structure_idx;
        	return ranks;
        }
    }


    int32_t structure_size() const
    {
        return self().leaf_size(0);
    }


    const auto* leaf_structure() const
    {
        auto& self = this->self();
        return self.ctr().template getPackedStruct<IntList<StructureStreamIdx, 1>>(self.leaf());
    }

    int32_t symbol_idx(int32_t stream, int32_t position) const
    {
        auto& self = this->self();

        auto s = self.leaf_structure();

        if (s != nullptr)
        {
        	return self.leaf_structure()->selectFW(position + 1, stream).idx();
        }
        else {
        	return 0;
        }
    }


MEMORIA_V1_ITERATOR_PART_END

#define M_TYPE      MEMORIA_V1_ITERATOR_TYPE(v1::btfl::IteratorSkipName)
#define M_PARAMS    MEMORIA_V1_ITERATOR_TEMPLATE_PARAMS


#undef M_TYPE
#undef M_PARAMS

}}