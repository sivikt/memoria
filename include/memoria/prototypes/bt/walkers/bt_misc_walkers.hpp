
// Copyright Victor Smirnov 2013-2015.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef _MEMORIA_PROTOTYPES_BALANCEDTREE_MISC_WALKERS_HPP
#define _MEMORIA_PROTOTYPES_BALANCEDTREE_MISC_WALKERS_HPP

#include <memoria/prototypes/bt/walkers/bt_find_walkers.hpp>
#include <memoria/core/packed/tools/packed_tools.hpp>

namespace memoria {
namespace bt      {






template <typename MyType, typename BranchPath, typename LeafPath>
class LeveledNodeWalkerBase {

    struct LeafStreamFn {
        template <Int StreamIdx, typename... Args>
        using RtnFnType = auto(Args...) -> decltype(
                std::declval<MyType>().template leafStream<StreamIdx>(std::declval<Args>()...)
        );

        template <Int StreamIdx, typename Fn, typename... Args>
        using RtnType = typename FnTraits<RtnFnType<StreamIdx, Fn, Args...>>::RtnType;


        MyType& walker_;

        LeafStreamFn(MyType& walker): walker_(walker) {}

        template <Int StreamIdx, typename Stream, typename... Args>
        RtnType<StreamIdx, const Stream*, Args...>
        stream(const Stream* stream, Args&&... args)
        {
            return walker_.template leafStream<StreamIdx>(stream, args...);
        }
    };


    struct NonLeafStreamFn {
        template <Int StreamIdx, typename... Args>
        using RtnFnType = auto(Args...) -> decltype(
                std::declval<MyType>().template nonLeafStream<StreamIdx>(std::declval<Args>()...)
        );

        template <Int StreamIdx, typename Fn, typename... Args>
        using RtnType = typename FnTraits<RtnFnType<StreamIdx, Fn, Args...>>::RtnType;


        MyType& walker_;

        NonLeafStreamFn(MyType& walker): walker_(walker) {}

        template <Int StreamIdx, typename Stream, typename... Args>
        RtnType<StreamIdx, const Stream*, Args...>
        stream(const Stream* stream, Args&&... args)
        {
            return walker_.template nonLeafStream<StreamIdx>(stream, args...);
        }
    };

    template <typename T, typename... Args>
    using BranchRtnFnType = auto(Args...) -> decltype(
        std::declval<T>().template processStream<BranchPath>(std::declval<Args>()...)
    );

    template <typename T, typename Fn, typename... Args>
    using BranchRtnType = typename FnTraits<BranchRtnFnType<T, Fn, Args...>>::RtnType;

    template <typename T, typename... Args>
    using LeafRtnFnType = auto(Args...) -> decltype(
        std::declval<T>().template processStream<LeafPath>(std::declval<Args>()...)
    );

    template <typename T, typename Fn, typename... Args>
    using LeafRtnType = typename FnTraits<LeafRtnFnType<T, Fn, Args...>>::RtnType;

public:

    template <typename NodeTypes, typename... Args>
    auto treeNode(const bt::LeafNode<NodeTypes>* node, Args&&... args)
        -> LeafRtnType<const bt::LeafNode<NodeTypes>, LeafStreamFn, Args...>
    {
        return node->template processStream<LeafPath>(LeafStreamFn(self()), args...);
    }

    template <typename NodeTypes, typename... Args>
    auto treeNode(const bt::BranchNode<NodeTypes>* node, Args&&... args)
        -> BranchRtnType<const bt::BranchNode<NodeTypes>, NonLeafStreamFn, Args...>
    {
        return node->template processStream<BranchPath>(NonLeafStreamFn(self()), args...);
    }

    template <typename NodeTypes, typename... Args>
    auto treeNode(bt::LeafNode<NodeTypes>* node, Args&&... args)
        -> LeafRtnType<bt::LeafNode<NodeTypes>, LeafStreamFn, Args...>
    {
        return node->template processStream<LeafPath>(LeafStreamFn(self()), args...);
    }

    template <typename NodeTypes, typename... Args>
    auto treeNode(bt::BranchNode<NodeTypes>* node, Args&&... args)
        -> BranchRtnType<bt::BranchNode<NodeTypes>, NonLeafStreamFn, Args...>
    {
        return node->template processStream<BranchPath>(NonLeafStreamFn(self()), args...);
    }

    MyType& self() {return *T2T<MyType*>(this);}
    const MyType& self() const {return *T2T<const MyType*>(this);}
};




template <typename MyType, typename BranchPath, typename LeafPath>
struct NodeWalkerBase {
private:

    template <typename T, typename... Args>
    using BranchRtnFnType = auto(Args...) -> decltype(
            std::declval<T>().template processStream<BranchPath>(std::declval<Args>()...)
    );

    template <typename T, typename Fn, typename... Args>
    using BranchRtnType = typename FnTraits<BranchRtnFnType<T, Fn, Args...>>::RtnType;


    template <typename T, typename... Args>
    using LeafRtnFnType = auto(Args...) -> decltype(
            std::declval<T>().template processStream<LeafPath>(std::declval<Args>()...)
    );

    template <typename T, typename Fn, typename... Args>
    using LeafRtnType = typename FnTraits<LeafRtnFnType<T, Fn, Args...>>::RtnType;

public:
    template <typename NodeTypes, typename... Args>
    auto treeNode(bt::LeafNode<NodeTypes>* node, Args&&... args)
        -> LeafRtnType<bt::LeafNode<NodeTypes>, MyType, Args...>
    {
        return node->template processStream<LeafPath>(self(), std::forward<Args>(args)...);
    }

    template <typename NodeTypes, typename... Args>
    auto treeNode(bt::BranchNode<NodeTypes>* node, Args&&... args)
        -> BranchRtnType<bt::BranchNode<NodeTypes>, MyType, Args...>
    {
        return node->template processStream<BranchPath>(self(), std::forward<Args>(args)...);
    }

    template <typename NodeTypes, typename... Args>
    auto treeNode(const bt::LeafNode<NodeTypes>* node, Args&&... args)
        -> LeafRtnType<const bt::LeafNode<NodeTypes>, MyType, Args...>
    {
        return node->template processStream<LeafPath>(self(), std::forward<Args>(args)...);
    }

    template <typename NodeTypes, typename... Args>
    auto treeNode(const bt::BranchNode<NodeTypes>* node, Args&&... args)
        -> BranchRtnType<const bt::BranchNode<NodeTypes>, MyType, Args...>
    {
        return node->template processStream<BranchPath>(self(), std::forward<Args>(args)...);
    }

    MyType& self() {return *T2T<MyType*>(this);}
    const MyType& self() const {return *T2T<const MyType*>(this);}
};



template <Int Stream, typename SubstreamsIdxList>
struct SubstreamsSetNodeFn {

private:

    template <typename T, typename... Args>
    using LeafRtnFnType = auto(Args...) -> decltype(
            std::declval<T>().template processSubstreamsByIdx<Stream, SubstreamsIdxList>(std::declval<Args>()...)
    );

    template <typename T, typename... Args>
    using LeafRtnType = typename FnTraits<LeafRtnFnType<T, Args...>>::RtnType;

public:
    template <typename NodeTypes, typename... Args>
    auto treeNode(bt::LeafNode<NodeTypes>* node, Args&&... args)
        -> LeafRtnType<bt::LeafNode<NodeTypes>, Args...>
    {
        return node->template processSubstreamsByIdx<Stream, SubstreamsIdxList>(std::forward<Args>(args)...);
    }


    template <typename NodeTypes, typename... Args>
    auto treeNode(const bt::LeafNode<NodeTypes>* node, Args&&... args)
        -> LeafRtnType<const bt::LeafNode<NodeTypes>, Args...>
    {
        return node->template processSubstreamsByIdx<Stream, SubstreamsIdxList>(std::forward<Args>(args)...);
    }

//
//    MyType& self() {return *T2T<MyType*>(this);}
//    const MyType& self() const {return *T2T<const MyType*>(this);}
};



struct GetLeafValuesFn {

    template <typename T, typename... Args>
    using FnType = auto (Args...)-> decltype(std::declval<T>().get_values(std::declval<Args>()...));

    template <typename T, typename... Args>
    using RtnType = typename FnTraits<FnType<typename std::remove_reference<T>::type, Args...>>::RtnType;


	template <typename StreamType, typename... Args>
	auto stream(const StreamType* obj, Args&&... args) -> RtnType<const StreamType, Args...>
	{
		return obj->get_values(std::forward<Args>(args)...);
	}
};



struct SetLeafValuesFn {

    template <typename T, typename... Args>
    using FnType = auto (Args...)-> decltype(std::declval<T>().set_values(std::declval<Args>()...));

    template <typename T, typename... Args>
    using RtnType = typename FnTraits<FnType<typename std::remove_reference<T>::type, Args...>>::RtnType;


	template <typename StreamType, typename... Args>
	auto stream(const StreamType* obj, Args&&... args) -> RtnType<const StreamType, Args...>
	{
		return obj->set_values(std::forward<Args>(args)...);
	}
};


}
}

#endif
