
// Copyright Victor Smirnov 2011-2013.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_PROTOTYPES_BALANCEDTREE_MACROS1_HPP
#define _MEMORIA_PROTOTYPES_BALANCEDTREE_MACROS1_HPP

#include <memoria/core/container/macros.hpp>

#define MEMORIA_BT_MODEL_BASE_CLASS_NO_CTOR_BEGIN(BTreeCtrBaseClassName)                \
template <                                                                              \
        typename TypesType                                                              \
>                                                                                       \
class BTreeCtrBaseClassName: public CtrBase<TypesType> {                                \
                                                                                        \
    typedef BTreeCtrBaseClassName<TypesType>                              ThisType;     \
    typedef CtrBase<TypesType>                                            Base;         \
    typedef Ctr<TypesType>                                                MyType;       \
                                                                                        \
    template <typename, typename, typename > friend class CtrPart;                      \
public:

#define MEMORIA_BT_MODEL_BASE_CLASS_BEGIN(BTreeCtrBaseClassName)                        \
    MEMORIA_BT_MODEL_BASE_CLASS_NO_CTOR_BEGIN(BTreeCtrBaseClassName)                    \
                                                                                        \
    BTreeCtrBaseClassName(const CtrInitData& data): Base(data) {}                       \



#define MEMORIA_BT_MODEL_BASE_CLASS_END                                                 \
private:                                                                                \
    MyType& self() {                                                                    \
        return *static_cast<MyType*>(this);                                             \
    }                                                                                   \
    const MyType& self() const {                                                        \
        return *static_cast<const MyType*>(this);                                       \
    }                                                                                   \
};



#define MEMORIA_BT_ITERATOR_BASE_CLASS_NO_CTOR_BEGIN(BTreeIteratorBaseClassName)        \
template <                                                                              \
        typename TypesType                                                              \
>                                                                                       \
class BTreeIteratorBaseClassName: public IteratorBase<TypesType>                        \
{                                                                                       \
    typedef IteratorBase<TypesType>                                             Base;   \
    typedef BTreeIteratorBaseClassName<TypesType>                               ThisType;\
    template <typename, typename, typename> friend class CtrPart;                       \
    template <typename, typename, typename> friend class IterPart;                      \
public:                                                                                 \
    typedef Iter<TypesType>                                                     MyType;



        
#define MEMORIA_BT_ITERATOR_BASE_CLASS_BEGIN(BTreeIteratorBaseClassName)                \
MEMORIA_BT_ITERATOR_BASE_CLASS_NO_CTOR_BEGIN(BTreeIteratorBaseClassName)                \
    BTreeIteratorBaseClassName(): Base() {}


#define MEMORIA_BT_ITERATOR_BASE_CLASS_END                                              \
    MyType& self() {                                                                    \
        /*return *static_cast<MyType*>(this);*/                                         \
        return *(MyType*)(this);                                                        \
    }                                                                                   \
                                                                                        \
    const MyType& self() const {                                                        \
        /*return *static_cast<const MyType*>(this);*/                                   \
        return *(MyType*)(this);                                                        \
    }                                                                                   \
};



#define MEMORIA_FN_WRAPPER(WrapperName, TargetMethod)   \
struct WrapperName {                                    \
    MyType& me_;                                        \
    WrapperName(MyType& v): me_(v) {}                   \
    template <typename T, typename... Args>             \
    void treeNode(T&& arg, Args&&... args)                \
    {                                                   \
        me_.TargetMethod(std::forward<T>(arg), std::forward<Args>(args)...);\
    }                                                   \
}

#define MEMORIA_FN_WRAPPER_RTN(WrapperName, TargetMethod, ReturnType_)\
struct WrapperName {                                    \
    typedef ReturnType_ ReturnType;                     \
    MyType& me_;                                        \
    WrapperName(MyType& v): me_(v) {}                   \
    template <typename T, typename... Args>             \
    ReturnType treeNode(T arg, Args&&... args)          \
    {                                                   \
        return me_.TargetMethod(arg, std::forward<Args>(args)...);\
    }                                                   \
}

#define MEMORIA_CONST_FN_WRAPPER(WrapperName, TargetMethod) \
struct WrapperName {                                        \
    const MyType& me_;                                      \
    WrapperName(const MyType& v): me_(v) {}                 \
    template <typename T, typename... Args>                 \
    void treeNode(T&& arg, Args&&... args) const              \
    {                                                       \
        me_.TargetMethod(std::forward<T>(arg), std::forward<Args>(args)...);\
    }                                                       \
}

#define MEMORIA_CONST_FN_WRAPPER_RTN(WrapperName, TargetMethod, ReturnType_)\
struct WrapperName {                                    \
    typedef ReturnType_ ReturnType;                     \
    const MyType& me_;                                  \
    WrapperName(const MyType& v): me_(v) {}             \
    template <typename T, typename... Args>             \
    ReturnType treeNode(T&& arg, Args&&... args) const    \
    {                                                   \
        return me_.TargetMethod(std::forward<T>(arg), std::forward<Args>(args)...);\
    }                                                   \
}




#define MEMORIA_CONST_STATIC_FN_WRAPPER_RTN(WrapperName, TargetMethod, ReturnType_)\
struct WrapperName {                                    \
    typedef ReturnType_ ReturnType;                     \
    const MyType& me_;                                  \
    WrapperName(const MyType& v): me_(v) {}             \
    template <typename T, typename... Args>             \
    ReturnType treeNode(const T*, Args&&... args) const \
    {                                                   \
        return me_.template TargetMethod<T>(std::forward<Args>(args)...);\
    }                                                   \
}


#define MEMORIA_DECLARE_NODE_FN(WrapperName, NodeMethodName)\
struct WrapperName {                                        \
    template <typename T, typename... Args>                 \
    void treeNode(T&& node, Args&&... args) const             \
    {                                                       \
        node->NodeMethodName(std::forward<Args>(args)...);\
    }                                                       \
}

#define MEMORIA_DECLARE_NODE2_FN(WrapperName, NodeMethodName)\
struct WrapperName {                                        \
    template <typename T, typename... Args>                 \
    void treeNode(T&& node1, T&& node2, Args&&... args) const   \
    {                                                       \
        node1->NodeMethodName(node2, std::forward<Args>(args)...);\
    }                                                       \
}

#define MEMORIA_DECLARE_NODE_FN_RTN(WrapperName, NodeMethodName, ReturnType_) \
struct WrapperName {                                        \
    typedef ReturnType_ ReturnType;                         \
    template <typename T, typename... Args>                 \
    ReturnType treeNode(T&& node, Args&&... args) const       \
    {                                                       \
        return node->NodeMethodName(std::forward<Args>(args)...);\
    }                                                       \
}

#define MEMORIA_DECLARE_NODE2_FN_RTN(WrapperName, NodeMethodName, ReturnType_) \
struct WrapperName {                                        \
    typedef ReturnType_ ReturnType;                         \
    template <typename T, typename... Args>                 \
    ReturnType treeNode(T&& node1, T&& node2, Args&&... args) const \
    {                                                       \
        return node1->NodeMethodName(node2, std::forward<Args>(args)...);\
    }                                                       \
}



#endif
