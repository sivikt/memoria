
// Copyright Victor Smirnov 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_BALANCEDTREE_MODEL_INSERT_BATCH_HPP
#define _MEMORIA_PROTOTYPES_BALANCEDTREE_MODEL_INSERT_BATCH_HPP

#include <memoria/prototypes/balanced_tree/baltree_tools.hpp>
#include <memoria/core/container/macros.hpp>

#include <vector>

namespace memoria {

using namespace memoria::balanced_tree;
using namespace memoria::core;

using namespace std;

MEMORIA_CONTAINER_PART_BEGIN(memoria::balanced_tree::InsertBatchName)

    typedef typename Base::Types                                                Types;
    typedef typename Base::Allocator                                            Allocator;

    typedef typename Base::ID                                                   ID;
    
    typedef typename Types::NodeBase                                            NodeBase;
    typedef typename Types::NodeBaseG                                           NodeBaseG;
    typedef typename Base::TreeNodePage                                         TreeNodePage;
    typedef typename Base::Iterator                                             Iterator;

    typedef typename Base::NodeDispatcher                                       NodeDispatcher;
    typedef typename Base::RootDispatcher                                       RootDispatcher;
    typedef typename Base::LeafDispatcher                                       LeafDispatcher;
    typedef typename Base::NonLeafDispatcher                                    NonLeafDispatcher;


    typedef typename Base::Key                                                  Key;
    typedef typename Base::Value                                                Value;
    typedef typename Base::Element                                              Element;

    typedef typename Base::Metadata                                             Metadata;

    typedef typename Types::Accumulator                                         Accumulator;
    typedef typename Types::Position 											Position;

    typedef typename Base::TreePath                                             TreePath;
    typedef typename Base::TreePathItem                                         TreePathItem;

    static const Int Indexes                                                    = Types::Indexes;
    static const Int Streams                                                    = Types::Streams;

    struct NonLeafNodeKeyValuePair
    {
        Accumulator keys;
        ID          value;
        BigInt      key_count;
    };

    struct ISubtreeProvider
    {
        typedef MyType                      CtrType;

        virtual NonLeafNodeKeyValuePair getKVPair(BigInt begin, BigInt count, Int level)    	= 0;
        virtual BigInt				    getTotalKeyCount()                          			= 0;
        virtual Position				getTotalSize()       									= 0;
        virtual Position				getTotalInserted()       								= 0;

        virtual Position				remainder()       										= 0;

        virtual ISubtreeProvider&       getProvider() {return *this;}

        virtual Accumulator insertIntoLeaf(NodeBaseG& leaf)  											= 0;
        virtual Accumulator insertIntoLeaf(NodeBaseG& leaf, const Position& from)  						= 0;
        virtual Accumulator insertIntoLeaf(NodeBaseG& leaf, const Position& from, const Position& size)	= 0;
    };

    class AbstractSubtreeProviderBase: public ISubtreeProvider {

    	MyType& ctr_;

    public:
    	AbstractSubtreeProviderBase(MyType& ctr): ctr_(ctr) {}

    	virtual NonLeafNodeKeyValuePair getKVPair(BigInt begin, BigInt total, Int level)
    	{
    		BigInt local_count = 0;
    		return BuildTree(begin, local_count, total, level - 1);
    	}

    	MyType& ctr() {
    		return ctr_;
    	}

    	const MyType& ctr() const {
    		return ctr_;
    	}


    private:
    	NonLeafNodeKeyValuePair BuildTree(BigInt start, BigInt& count, const BigInt total, Int level)
    	{
    		NonLeafNodeKeyValuePair pair;
    		pair.key_count = 0;



    		if (level > 0)
    		{
    			Int max_keys = ctr_.getMaxKeyCountForNode(false, level == 0, level);

    			// FIXME: buffer size can be too small
    			NonLeafNodeKeyValuePair children[2000];

    			Int local = 0;
    			for (Int c = 0; c < max_keys && count < total; c++, local++)
    			{
    				children[c]     =  BuildTree(start, count, total, level - 1);
    				pair.key_count  += children[c].key_count;
    			}

    			NodeBaseG node = ctr_.createNode(level, false, false);

    			setINodeData(children, node, local);

    			pair.keys  = ctr_.getMaxKeys(node);
    			pair.value = node->id();
    		}
    		else
    		{
    			NodeBaseG node = ctr_.createNode(level, false, true);

    			count++;
    			pair.keys = this->insertIntoLeaf(node);

    			pair.value      =  node->id();
    			pair.key_count  += 1;
    		}

    		return pair;
    	}

    	template <typename PairType, typename ParentPairType>
    	struct SetNodeValuesFn
    	{
    		PairType*       pairs_;
    		Int             count_;

    		ParentPairType  total_;

    		SetNodeValuesFn(PairType* pairs, Int count): pairs_(pairs), count_(count) {}

    		template <typename Node>
    		void treeNode(Node* node)
    		{
    			for (Int c = 0; c < count_; c++)
    			{
    				node->setKeys(c, pairs_[c].keys);

    				node->value(c) = pairs_[c].value;
    			}

    			node->set_children_count(count_);

    			node->reindex();

    			total_.keys = node->maxKeys();

    			total_.value = node->id();
    		}
    	};

    	template <typename PairType>
    	NonLeafNodeKeyValuePair setINodeData(PairType* data, NodeBaseG& node, Int count)
    	{
    		SetNodeValuesFn<PairType, NonLeafNodeKeyValuePair> fn(data, count);
    		NonLeafDispatcher::dispatch(node, fn);
    		return fn.total_;
    	}
    };



    template <typename Node>
    class LayoutManager: public INodeLayoutManager {
    	Int block_size_;
    public:
    	LayoutManager(Int block_size): block_size_(block_size) {}

    	virtual Int getNodeCapacity(const Int* sizes, Int stream)
    	{
    		return Node::capacity(block_size_, sizes, stream);
    	}
    };

    struct GetTotalNodesFn {
    	typedef Int ReturnType;

    	template <typename Node>
    	ReturnType treeNode(const Node*, Int block_size, const Int* sizes, Int stream)
    	{
    		return Node::capacity(block_size, sizes, stream);
    	}
    };

    class LeafLayoutManager: public INodeLayoutManager {
    	Int block_size_;
    public:
    	LeafLayoutManager(Int block_size): block_size_(block_size) {}
    	virtual Int getNodeCapacity(const Int* sizes, Int stream)
    	{
    		return LeafDispatcher::dispatchStatic2Rtn(false, true, GetTotalNodesFn(), block_size_, sizes, stream);
    	}
    };



    class DefaultSubtreeProvider: public AbstractSubtreeProviderBase {

    	typedef AbstractSubtreeProviderBase 				ProviderBase;

    	MyType& 	ctr_;
    	Position 	total_;
    	Position 	inserted_;

    	ISource& 	data_source_;

    public:
    	DefaultSubtreeProvider(MyType& ctr, const Position& total, ISource& data_source):
    		ProviderBase(ctr),
    		ctr_(ctr),
    		total_(total),
    		data_source_(data_source)
    	{}

    	virtual BigInt getTotalKeyCount()
    	{
    		LeafLayoutManager manager(ctr_.getRootMetadata().page_size());
    		return data_source_.getTotalNodes(&manager);
    	}

    	virtual Position getTotalSize()
    	{
    		return total_;
    	}

    	virtual Position getTotalInserted()
    	{
    		return inserted_;
    	}

    	virtual Position remainder()
    	{
    		return total_ - inserted_;
    	}

    	struct InsertIntoLeafFn {

    		typedef Accumulator ReturnType;

    		template <typename Node>
    		Accumulator treeNode(
    				Node* node,
    				DefaultSubtreeProvider* provider,
    				const Position* pos,
    				const Position* remainder
    		)
    		{
    			Position size;
    			if (remainder->lteAll(node->capacities()))
    			{
    				size = *remainder;
    			}
    			else {
    				size = node->capacities();
    			}

    			LayoutManager<Node> manager(node->page_size());

    			provider->data_source_.newNode(&manager);

    			node->insert(provider->data_source_, *pos, size);

    			node->reindex();

    			provider->inserted_ += size;

    			return node->sum(*pos, (*pos) + size);
    		}
    	};

    	virtual Accumulator insertIntoLeaf(NodeBaseG& leaf)
    	{
    		Position pos;
    		Position remainder = this->remainder();
    		return LeafDispatcher::dispatchRtn(leaf, InsertIntoLeafFn(), this, &pos, &remainder);
    	}

    	virtual Accumulator insertIntoLeaf(NodeBaseG& leaf, const Position& from)
    	{
    		Position remainder = this->remainder();
    		return LeafDispatcher::dispatchRtn(leaf, InsertIntoLeafFn(), this, &from, &remainder);
    	}

    	virtual Accumulator insertIntoLeaf(NodeBaseG& leaf, const Position& from, const Position& size)
    	{
    		return LeafDispatcher::dispatchRtn(leaf, InsertIntoLeafFn(), this, &from, &size);
    	}
    };






MEMORIA_CONTAINER_PART_END

#define M_TYPE      MEMORIA_CONTAINER_TYPE(memoria::balanced_tree::InsertBatchName)
#define M_PARAMS    MEMORIA_CONTAINER_TEMPLATE_PARAMS



#undef M_TYPE
#undef M_PARAMS

} //memoria



#endif
