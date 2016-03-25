
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

#include <memoria/v1/core/types/types.hpp>
#include <memoria/v1/core/tools/dump.hpp>

#include <memoria/v1/containers/seq_dense/seqd_names.hpp>
#include <memoria/v1/containers/seq_dense/seqd_tools.hpp>
#include <memoria/v1/core/container/iterator.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include <memoria/v1/prototypes/bt/bt_macros.hpp>

#include <memoria/v1/core/packed/array/packed_fse_bitmap.hpp>

namespace memoria {
namespace v1 {


MEMORIA_V1_ITERATOR_PART_BEGIN(v1::seq_dense::IterMiscName)
public:
    typedef Ctr<typename Types::CtrTypes>                                       Container;


    typedef typename Base::Allocator                                            Allocator;
    typedef typename Base::NodeBaseG                                            NodeBaseG;

    typedef typename Container::BranchNodeEntry                                 BranchNodeEntry;
    typedef typename Container::Iterator                                        Iterator;

    typedef typename Container::Types::Pages::LeafDispatcher                    LeafDispatcher;
    typedef typename Container::Position                                        Position;

    using SymbolsSubstreamPath = typename Container::Types::SymbolsSubstreamPath;

    using typename Base::CtrSizeT;

    template <Int Stream>
    using InputTupleAdapter = typename Container::Types::template InputTupleAdapter<Stream>;

    static const Int BitsPerSymbol  = Container::Types::BitsPerSymbol;
    static const Int Symbols        = Container::Types::Symbols;


    Int symbol() const
    {
        auto& self  = this->self();
        return std::get<0>(self.ctr().template read_leaf_entry<IntList<1>>(self.leaf(), self.idx()));
    }


    void setSymbol(Int symbol)
    {
        auto& self  = this->self();

        self.ctr().template update_stream_entry<0, IntList<1>>(self, std::make_tuple(symbol));
    }


    void insert_symbol(Int symbol)
    {
        MEMORIA_V1_ASSERT(symbol, <, Symbols);

        auto& self  = this->self();
        auto& ctr   = self.ctr();

        ctr.insert_entry(
                self,
                InputTupleAdapter<0>::convert(0, symbol)
        );

        self.skipFw(1);
    }


//    template <typename T>
//    struct ReadWalker {
//        T& data_;
//        CtrSizeT processed_ = 0;
//        const CtrSizeT max_;
//
//        ReadWalker(T& data): data_(data), max_(data.size()) {}
//
//        template <typename NodeTypes>
//        Int treeNode(const LeafNode<NodeTypes>* leaf, Int start)
//        {
//            return std::get<0>(leaf->template processSubstreams<IntList<0, 0>>(*this, start));
//        }
//
//        template <typename StreamObj>
//        Int stream(const StreamObj* obj, Int start)
//        {
//            if (obj != nullptr)
//            {
//                Int size        = obj->size();
//                Int remainder   = size - start;
//
//                Int to_read = (processed_ + remainder < max_) ? remainder : (max_ - processed_);
//
//                obj->read(&data_, start, processed_, to_read);
//
//                return to_read;
//            }
//            else {
//                return 0;
//            }
//        }
//
//        void start_leaf() {}
//
//        void end_leaf(Int skip) {
//            processed_ += skip;
//        }
//
//        CtrSizeT result() const {
//            return processed_;
//        }
//
//        bool stop() const {
//            return processed_ >= max_;
//        }
//    };



//    BigInt read(SymbolsBuffer<BitsPerSymbol>& data)
//    {
//        auto& self = this->self();
//        ReadWalker<SymbolsBuffer<BitsPerSymbol>> target(data);
//
//        return self.ctr().readStream2(self, target);
//    }
//
//
//    void insert_symbols(SymbolsBuffer<BitsPerSymbol>& data)
//    {
//        auto& self = this->self();
//        auto& model = self.ctr();
//
//        auto& leaf = self.leaf();
//
//        seq_dense::SymbolsInputBufferProvider<BitsPerSymbol> provider(data);
//
//        auto result = model.insertBuffers(leaf, self.idx(), provider);
//
//        self.leaf() = result.leaf();
//        self.idx() = result.position();
//
//        self.refresh();
//
//        model.markCtrUpdated();
//    }



    template <typename Seq>
    class ReadAdaptor {
    	const Seq& seq_;
    	Int pos_ = 0;

    public:
        ReadAdaptor(const Seq& seq): seq_(seq) {}

        template <typename V>
        void process(StreamTag<0>, StreamTag<1>, Int start, Int end, V&& value)
        {
        	for (Int c = start; c < end; c++)
        	{
        		auto sym1 = seq_.symbol(pos_ + c - start);
        		auto sym2 = value.symbol(c);

        		if (sym1 != sym2) {
        			cout << "Mismatch! " << sym1 << " " << sym2 << " " << (pos_ + c - start) << endl;
        		}
        	}

        	pos_ += (end - start);
        }
    };


//    template <typename Seq>
//    auto read(Seq&& seq)
//    {
//        auto& self = this->self();
//
//        ReadAdaptor<Seq> adaptor(&seq);
//
//        return self.ctr().template read_single_substream2<IntList<0, 1>>(self, length, adaptor);
//    }

    template <typename Seq>
    auto compare(Seq&& seq)
    {
    	auto& self = this->self();

    	ReadAdaptor<Seq> adaptor(seq);

    	return self.ctr().template read_single_substream2<IntList<0, 1>>(self, seq.size(), adaptor);
    }


    void check(const char* source = nullptr) const
    {
        auto& self = this->self();

        auto tmp = self;

        tmp.refresh();

        if (self.cache() != tmp.cache())
        {
            throw TestException(
                    source != nullptr ? source : MA_SRC,
                    SBuf()<<"Iterator cache mismatch: having: "<<self.cache()<<", should be: "<<tmp.cache()
            );
        }
    }

    template <typename Provider>
    CtrSizeT bulk_insert(Provider&& provider)
    {
    	auto& self = this->self();
    	return self.ctr().insert(self, std::forward<Provider>(provider));
    }


MEMORIA_V1_ITERATOR_PART_END


#define M_TYPE      MEMORIA_V1_ITERATOR_TYPE(v1::seq_dense::IterMiscName)
#define M_PARAMS    MEMORIA_V1_ITERATOR_TEMPLATE_PARAMS




#undef M_TYPE
#undef M_PARAMS


}}