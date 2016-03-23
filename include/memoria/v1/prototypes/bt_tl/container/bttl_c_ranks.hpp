
// Copyright Victor Smirnov 2015+.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memoria/v1/prototypes/bt_tl/bttl_names.hpp>
#include <memoria/v1/core/container/container.hpp>
#include <memoria/v1/core/container/macros.hpp>

#include <memoria/v1/prototypes/bt_tl/bttl_tools.hpp>


#include <vector>

namespace memoria {
namespace v1 {

MEMORIA_V1_CONTAINER_PART_BEGIN(v1::bttl::RanksName)
public:
    using Types             = typename Base::Types;
    using Iterator          = typename Base::Iterator;

protected:
    using NodeBaseG         = typename Types::NodeBaseG;
    using NodeDispatcher    = typename Types::Pages::NodeDispatcher;
    using LeafDispatcher    = typename Types::Pages::LeafDispatcher;
    using BranchDispatcher  = typename Types::Pages::BranchDispatcher;

    using Key                   = typename Types::Key;
    using Value                 = typename Types::Value;
    using CtrSizeT              = typename Types::CtrSizeT;

    using BranchNodeEntry       = typename Types::BranchNodeEntry;
    using Position              = typename Types::Position;

    static const Int Streams            = Types::Streams;
    static const Int SearchableStreams  = Types::SearchableStreams;

    using PageUpdateMgt     = typename Types::PageUpdateMgr;

    using LeafPrefixRanks   = typename Types::LeafPrefixRanks;

    template <Int StreamIdx>
    using LeafSizesSubstreamPath = typename Types::template LeafSizesSubstreamPath<StreamIdx>;

    using AnchorValueT  = core::StaticVector<CtrSizeT, Streams - 1>;
    using AnchorPosT    = core::StaticVector<Int, Streams - 1>;


public:
    auto total_counts() const
    {
        auto& self = this->self();

        NodeBaseG root = self.getRoot();

        auto root_counts = self.count_items(root);

        root_counts[0] = self.sizes()[0];

        return root_counts;
    }

    Position node_extents(const NodeBaseG& node) const
    {
        auto& self = this->self();

        auto counts = self.count_items(node);
        auto sizes  = self.node_stream_sizes(node);

        return counts - sizes;
    }


    //FIXME: handle PackedOOM correctly for branch nodes
    void add_to_stream_counter(NodeBaseG node, Int stream, Int idx, CtrSizeT value)
    {
        auto& self = this->self();

        if (value != 0)
        {
            AddToStreamCounter fn;

            self.updatePageG(node);

            self.process_count_substreams(node, stream, fn, idx, value);

            while (node->parent_id().isSet())
            {
                NodeBaseG parent = self.getNodeParentForUpdate(node);
                Int parent_idx   = node->parent_idx();

                self.updatePageG(parent);

                self.process_count_substreams(parent, stream, fn, parent_idx, value);

                node = parent;
            }
        }
    }

    template <typename... Args>
    Position leafrank_(const NodeBaseG& leaf, const Position& sizes, const Position& extent, Int pos, const AnchorPosT& anchors = AnchorPosT(-1), const AnchorValueT& anchor_values = AnchorValueT(0)) const
    {
        auto& self = this->self();

        MEMORIA_V1_ASSERT_TRUE(sizes.gteAll(0));
        MEMORIA_V1_ASSERT_TRUE(extent.gteAll(0));

        LeafPrefixRanks prefixes;

        self.compute_leaf_prefixes(leaf, extent, prefixes, anchors, anchor_values);

        return self.leafrank_(leaf, sizes, prefixes, pos, anchors, anchor_values);
    }


    template <typename... Args>
    Position leafrank_(const NodeBaseG& leaf, Position sizes, const LeafPrefixRanks& prefixes, Int pos, Args&&... args) const
    {
        if (pos >= sizes.sum()) {
            return sizes;
        }
        else
        {
            for (Int s = 0; s < Streams; s++)
            {
                Int sum = prefixes[s].sum();
                if (pos >= sum)
                {
                    return bttl::detail::ZeroRankHelper<0, Streams>::process(this, leaf, sizes, prefixes[s], pos, std::forward<Args>(args)...);
                }
                else {
                    sizes[s] = 0;
                }
            }

            return Position();
        }
    }


    template <Int Stream, typename... Args>
    Position _streamsrank_(const NodeBaseG& leaf, Args&&... args) const
    {
        return LeafDispatcher::dispatch(leaf, _StreamsRankFn<Stream>(), std::forward<Args>(args)...);
    }

    void compute_leaf_prefixes(const NodeBaseG& leaf, const Position& extent, LeafPrefixRanks& prefixes, const AnchorPosT& anchors = AnchorPosT(-1), const AnchorValueT& anchor_values = AnchorValueT(0)) const
    {
        const auto& self  = this->self();

        prefixes[Streams - 2][Streams - 1] = extent[Streams - 1];

        for (Int s = SearchableStreams - 1; s > 0; s--)
        {
            prefixes[s - 1] = prefixes[s];
            self.count_downstream_items(leaf, prefixes[s], prefixes[s - 1], s, extent[s], anchors, anchor_values);
            prefixes[s - 1][s] = extent[s];
        }
    }


protected:

    template <Int StreamIdx>
    struct ProcessCountSubstreamFn {
        template <typename Node, typename Fn, typename... Args>
        auto treeNode(Node* node, Fn&& fn, Args&&... args)
        {
            constexpr Int SubstreamIdx = Node::template StreamStartIdx<StreamIdx>::Value +
                                         Node::template StreamSize<StreamIdx>::Value - 1;

            return Node::Dispatcher::template dispatch<SubstreamIdx>(node->allocator(), std::forward<Fn>(fn), node->is_leaf(), std::forward<Args>(args)...);
        }

        template <typename Node, typename Fn, typename... Args>
        auto treeNode(const Node* node, Fn&& fn, Args&&... args)
        {
            constexpr Int SubstreamIdx = Node::template StreamStartIdx<StreamIdx>::Value +
                                         Node::template StreamSize<StreamIdx>::Value - 1;

            return Node::Dispatcher::template dispatch<SubstreamIdx>(node->allocator(), std::forward<Fn>(fn), node->is_leaf(), std::forward<Args>(args)...);
        }
    };


    template <Int StreamIdx, typename Fn, typename... Args>
    auto _process_count_substream(NodeBaseG& node, Fn&& fn, Args&&... args)
    {
        return NodeDispatcher::dispatch(
                node,
                ProcessCountSubstreamFn<StreamIdx>(),
                std::forward<Fn>(fn),
                std::forward<Args>(args)...
        );
    }

    template <Int StreamIdx, typename Fn, typename... Args>
    auto _process_count_substream(const NodeBaseG& node, Fn&& fn, Args&&... args) const
    {
        return NodeDispatcher::dispatch(
                node,
                ProcessCountSubstreamFn<StreamIdx>(),
                std::forward<Fn>(fn),
                std::forward<Args>(args)...
        );
    }



    struct ProcessCountSubstreamsFn {
        template <Int StreamIdx, typename CtrT, typename... Args2>
        auto process(CtrT&& ctr, NodeBaseG& node, Args2&&... args)
        {
            return ctr.template _process_count_substream<StreamIdx>(node, std::forward<Args2>(args)...);
        }

        template <Int StreamIdx, typename CtrT, typename... Args2>
        auto process(CtrT&& ctr, const NodeBaseG& node, Args2&&... args)
        {
            return ctr.template _process_count_substream<StreamIdx>(node, std::forward<Args2>(args)...);
        }
    };

    template <typename Fn, typename... Args>
    auto process_count_substreams(NodeBaseG& node, Int stream, Fn&& fn, Args&&... args)
    {
        return bt::ForEachStream<Streams - 2>::process(
                stream,
                ProcessCountSubstreamsFn(),
                self(),
                node,
                std::forward<Fn>(fn),
                std::forward<Args>(args)...
        );
    }

    template <typename Fn, typename... Args>
    auto process_count_substreams(const NodeBaseG& node, Int stream, Fn&& fn, Args&&... args) const
    {
        return bt::ForEachStream<Streams - 2>::process(
                stream,
                ProcessCountSubstreamsFn(),
                self(),
                node,
                std::forward<Fn>(fn),
                std::forward<Args>(args)...
        );
    }

    struct AddToStreamCounter {
        template <typename Stream>
        void stream(Stream* obj, bool leaf, Int idx, CtrSizeT value)
        {
            auto block = leaf? 0 : (Stream::Blocks - 1);
            obj->addValue(block, idx, value);
        }

        template <typename Stream>
        void stream(const Stream* obj, bool leaf, Int idx, CtrSizeT value)
        {
            throw Exception(MA_SRC, "Incorrect static method dispatching");
        }
    };


    struct GetStreamCounter {
        template <typename Stream>
        auto stream(const Stream* obj, bool leaf, Int idx)
        {
            auto block = leaf? 0 : (Stream::Blocks - 1);
            return obj->getValue(block, idx);
        }
    };

    CtrSizeT get_stream_counter(const NodeBaseG& node, Int stream, Int idx) const
    {
        auto& self = this->self();
        return self.process_count_substreams(node, stream, GetStreamCounter(), idx);
    }

    template <Int StreamIdx>
    CtrSizeT _get_stream_counter(const NodeBaseG& node, Int idx) const
    {
        auto& self = this->self();
        return self.template _process_count_substream<StreamIdx>(node, GetStreamCounter(), idx);
    }




    struct SetStreamCounter {
        template <typename Stream>
        auto stream(Stream* obj, bool leaf, Int idx, CtrSizeT value)
        {
            auto block = leaf? 0 : (Stream::Blocks - 1);
            auto current_value = obj->getValue(block, idx);

            obj->setValue(block, idx, value);

            return current_value;
        }
    };

    void set_stream_counter(NodeBaseG& node, Int stream, Int idx, CtrSizeT value)
    {
        auto& self = this->self();
        self.process_count_substreams(node, stream, SetStreamCounter(), idx, value);
    }

    struct FindOffsetFn {
        template <typename Stream>
        auto stream(const Stream* substream, bool leaf, Int idx)
        {
            auto block = leaf? 0 : (Stream::Blocks - 1);

            auto result = substream->findGTForward(block, 0, idx);
            return result.idx();
        }
    };



    Int find_offset(const NodeBaseG& node, Int stream, Int idx) const
    {
        MEMORIA_V1_ASSERT_TRUE(stream >= 0 && stream < Streams - 1);
        MEMORIA_V1_ASSERT(idx, >=, 0);

        return self().process_count_substreams(node, stream, FindOffsetFn(), idx);
    }



    struct CountStreamItemsFn {
        template <typename Stream>
        auto stream(const Stream* substream, bool leaf, Int end)
        {
            auto block = leaf? 0 : (Stream::Blocks - 1);

            return substream ? substream->sum(block, end) : 0;
        }
    };

    CtrSizeT count_items(const NodeBaseG& node, Int stream, Int end) const
    {
        return self().process_count_substreams(node, stream, CountStreamItemsFn(), end);
    }


    Position count_items(const NodeBaseG& node) const
    {
        auto& self = this->self();
        auto sizes = self.getNodeSizes(node);

        Position counts;

        counts[0] = self.node_stream_sizes(node)[0];

        for (Int s = 1; s < Streams; s++)
        {
            counts[s] = self.count_items(node, s - 1, sizes[s - 1]);
        }

        return counts;
    }









    template <Int Stream>
    struct _StreamsRankFn {
        template <typename NTypes, typename... Args, typename T1, typename T2>
        Position treeNode(const LeafNode<NTypes>* leaf, const Position& sizes, const Position& prefix, Int pos, T1&& anchors, T2&& anchor_values, Args&&... args)
        {
            bttl::detail::StreamsRankFn<
                LeafSizesSubstreamPath,
                CtrSizeT,
                Position
            > fn(sizes, prefix, pos, anchors, anchor_values);

            bttl::detail::StreamsRankHelper<Stream, SearchableStreams>::process(leaf, fn, std::forward<Args>(args)...);

            return fn.indexes_ + fn.prefix_;
        }

        template <typename NTypes, typename... Args>
        Position treeNode(const LeafNode<NTypes>* leaf, const Position& sizes, const Position& prefix, Int pos, Args&&... args)
        {
            bttl::detail::StreamsRankFn<
                LeafSizesSubstreamPath,
                CtrSizeT,
                Position
            > fn(sizes, prefix, pos);

            bttl::detail::StreamsRankHelper<Stream, SearchableStreams>::process(leaf, fn, std::forward<Args>(args)...);

            return fn.indexes_ + fn.prefix_;
        }
    };






private:

    struct CountStreamsItemsFn {

        Position prefix_;
        Position& sums_;
        Int stream_;
        CtrSizeT cnt_;

        const AnchorPosT& anchors_;
        const AnchorValueT& anchor_values_;

        CountStreamsItemsFn(const Position& prefix, Position& sums, Int stream, Int end, const AnchorPosT& anchors, const AnchorValueT& anchor_values):
            prefix_(prefix), sums_(sums), stream_(stream), cnt_(end), anchors_(anchors), anchor_values_(anchor_values)
        {}

        template <typename Stream>
        auto stream(const Stream* substream, CtrSizeT start, CtrSizeT end)
        {
            auto size = substream->size();
            auto limit = end < size ? end : size;
            auto sum = limit > start ? substream->sum(0, start, limit) : 0;

            return sum;
        }


        template <Int StreamIdx, typename Leaf>
        bool process(const Leaf* leaf)
        {
            if (StreamIdx >= stream_)
            {
                constexpr Int SubstreamIdx = Leaf::template StreamStartIdx<StreamIdx>::Value +
                                             Leaf::template StreamSize<StreamIdx>::Value - 1;

                auto start  = prefix_[StreamIdx];
                auto end    = prefix_[StreamIdx] + cnt_;

                auto sum = Leaf::Dispatcher::template dispatch<SubstreamIdx>(
                        leaf->allocator(),
                        *this,
                        start,
                        end
                );

                auto anchor = anchors_[StreamIdx];

                auto fix = (anchor >= start && anchor < end) ? anchor_values_[StreamIdx] : 0;

                sums_[StreamIdx + 1] += sum + fix;

                cnt_ = sum + fix;
            }

            return true;
        }


        template <typename NTypes>
        void treeNode(const LeafNode<NTypes>* leaf)
        {
            ForEach<0, SearchableStreams>::process(*this, leaf);
        }
    };


    void count_downstream_items(const NodeBaseG& leaf, const Position& prefix, Position& sums, Int stream, Int end, const AnchorPosT& anchors, const AnchorValueT& anchor_values) const
    {
        return LeafDispatcher::dispatch(leaf, CountStreamsItemsFn(prefix, sums, stream, end, anchors, anchor_values));
    }



MEMORIA_V1_CONTAINER_PART_END

#define M_TYPE      MEMORIA_V1_CONTAINER_TYPE(v1::bttl::RanksName)
#define M_PARAMS    MEMORIA_V1_CONTAINER_TEMPLATE_PARAMS



#undef M_PARAMS
#undef M_TYPE

}}