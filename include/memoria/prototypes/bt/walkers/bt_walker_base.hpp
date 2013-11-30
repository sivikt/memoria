
// Copyright Victor Smirnov 2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _MEMORIA_PROTOTYPES_BALANCEDTREE_WALKER_BASE_HPP
#define _MEMORIA_PROTOTYPES_BALANCEDTREE_WALKER_BASE_HPP

namespace memoria {
namespace bt1 	  {


template <typename Types, typename MyType>
class WalkerBase {
protected:
    typedef Iter<typename Types::IterTypes>                                     Iterator;
    typedef typename Types::IteratorPrefix										IteratorPrefix;

    typedef BigInt																Key;

    static const Int Streams                                                    = Types::Streams;

    SearchType search_type_ = SearchType::GT;

    BigInt sum_         = 0;
    BigInt target_      = 0;

    WalkDirection direction_;

    Int stream_;
    Int index_;

    IteratorPrefix prefix_;

    Int current_tree_level_;

    bool multistream_ = false;

    bool end_ = false;

private:
    struct OtherNonLeafStreamsProc {
    	MyType& walker_;

    	OtherNonLeafStreamsProc(MyType& walker): walker_(walker) {}

    	template <Int StreamIndex, typename StreamType>
    	void stream(const StreamType* stream, Int start, Int idx)
    	{
    		if (stream && (StreamIndex != walker_.current_stream()))
    		{
    			walker_.template postProcessOtherNonLeafStreams<StreamIndex>(stream, start, idx);
    		}
    	}
    };

    struct OtherLeafStreamsProc {
    	MyType& walker_;

    	OtherLeafStreamsProc(MyType& walker): walker_(walker) {}

    	template <Int StreamIndex, typename StreamType>
    	void stream(const StreamType* stream)
    	{
    		if (stream && (StreamIndex != walker_.current_stream()))
    		{
    			walker_.template postProcessOtherLeafStreams<StreamIndex>(stream);
    		}
    	}
    };

public:

    typedef Int                                                                 ReturnType;

    WalkerBase(Int stream, Int index, BigInt target):
        target_(target),
        stream_(stream),
        index_(index)
    {}

    const WalkDirection& direction() const {
        return direction_;
    }

    WalkDirection& direction() {
        return direction_;
    }

    void empty(Iterator& iter)
    {
        iter.idx() = 0;
    }

    BigInt sum() const {
        return sum_;
    }

    Int current_stream() const {
        return stream_;
    }

    Int index() const
    {
    	return index_;
    }

    const SearchType& search_type() const {
        return search_type_;
    }

    SearchType& search_type() {
        return search_type_;
    }

    void prepare(Iterator& iter)
    {
    	prefix_ = iter.cache().prefixes();
    }

    BigInt finish(Iterator& iter, Int idx)
    {
        iter.idx() = idx;

        iter.cache().prefixes() = prefix_;

        return sum_;
    }

    MyType& self()
    {
        return *T2T<MyType*>(this);
    }

    const MyType& self() const
    {
        return *T2T<const MyType*>(this);
    }

    template <Int StreamIdx, typename StreamType, typename Result>
    void postProcessStream(const StreamType*, Int, const Result& result) {}

    template <Int StreamIdx, typename StreamType>
    void postProcessOtherNonLeafStreams(const StreamType*, Int, Int) {}

    template <Int StreamIdx, typename StreamType>
    void postProcessOtherLeafStreams(const StreamType*, Int, Int) {}

    template <typename Node>
    void postProcessNode(const Node*, Int, Int) {}

    template <typename Node>
    ReturnType treeNode(const Node* node, BigInt start)
    {
    	current_tree_level_ = node->level();

        Int idx = node->find(stream_, self(), start);

        self().postProcessNode(node, start, idx);

        if (multistream_ && Streams > 1)
        {
        	if (current_tree_level_ > 0)
        	{
        		OtherNonLeafStreamsProc proc(self());
        		node->processAll(proc, start, idx);
        	}
        	else if (direction_ == WalkDirection::UP)
        	{
        		OtherLeafStreamsProc proc(self());
        		node->processAll(proc);
        	}
        }

        return idx;
    }
};


struct EmptyIteratorPrefixFn {
	template <typename StreamType, typename IteratorPrefix>
	void processNonLeafFw(const StreamType*, IteratorPrefix&, Int start, Int end, Int index, BigInt prefix)
	{}

	template <typename StreamType, typename IteratorPrefix>
	void processLeafFw(const StreamType*, IteratorPrefix&, Int start, Int end, Int index, BigInt prefix)
	{}

	template <typename StreamType, typename IteratorPrefix>
	void processNonLeafBw(const StreamType*, IteratorPrefix&, Int start, Int end, Int index, BigInt prefix)
	{}

	template <typename StreamType, typename IteratorPrefix>
	void processLeafBw(const StreamType*, IteratorPrefix&, Int start, Int end, Int index, BigInt prefix)
	{}
};


}
}

#endif