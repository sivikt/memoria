
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once

#include <memoria/v1/core/tools/config.hpp>

#if 0
#define MEMORIA_EXTERN_TREE(Key, Value, Indexes)            \
MEMORIA_TEMPLATE_EXTERN template class PackedSumTree<       \
    PackedTreeTypes<                                        \
        Key,                                                \
        Key,                                                \
        Value,                                              \
        v1::bt::Accumulators<Key, Indexes>, \
        Indexes                                             \
    >                                                       \
>;                                                          \
MEMORIA_TEMPLATE_EXTERN template class PackedTree<          \
    PackedTreeTypes<                                        \
        Key,                                                \
        Key,                                                \
        Value,                                              \
        v1::btree::Accumulators<Key, Indexes>,         \
        Indexes                                             \
    >                                                       \
>
#else
#define MEMORIA_EXTERN_TREE(Key, Value, Indexes)
#endif


#define MEMORIA_EXTERN_CTR(CollectionName,  CtrName)\
MEMORIA_TEMPLATE_EXTERN template class Ctr<CollectionName::Types<CtrName>::Type::CtrTypes>;

#define MEMORIA_EXTERN_ITER(CollectionName, CtrName)\
MEMORIA_TEMPLATE_EXTERN template class Iter<CollectionName::Types<CtrName>::Type::IterTypes>;

#define MEMORIA_EXTERN_CTR_BASE(CollectionName,  CtrName, BaseName) \
MEMORIA_TEMPLATE_EXTERN template class BaseName<CollectionName::Types<CtrName>::Type::CtrTypes >;


#define MEMORIA_EXTERN_ITER_BASE(CollectionName,  CtrName, BaseName) \
MEMORIA_TEMPLATE_EXTERN template class BaseName<CollectionName::Types<CtrName>::Type::IterTypes >;

#define MEMORIA_EXTERN_CONTAINER(CollectionName,  CtrName, CtrBaseName, IterBaseName)   \
MEMORIA_EXTERN_CTR(CollectionName,  CtrName)                                            \
MEMORIA_EXTERN_ITER(CollectionName,  CtrName)                                           \
MEMORIA_EXTERN_CTR_BASE(CollectionName,  CtrName, v1::CtrBase)               \
MEMORIA_EXTERN_CTR_BASE(CollectionName,  CtrName, CtrBaseName)                          \
MEMORIA_EXTERN_ITER_BASE(CollectionName,  CtrName, v1::IteratorBase)               \
MEMORIA_EXTERN_ITER_BASE(CollectionName,  CtrName, IterBaseName)                        \
MEMORIA_EXTERN_DATAPAGE(CollectionName,  CtrName, RootTypes, ANY_LEVEL)                 \
MEMORIA_EXTERN_DATAPAGE(CollectionName,  CtrName, RootLeafTypes, ANY_LEVEL)             \
MEMORIA_EXTERN_DATAPAGE(CollectionName,  CtrName, LeafTypes, ANY_LEVEL)                 \
MEMORIA_EXTERN_DATAPAGE(CollectionName,  CtrName, InternalTypes, ANY_LEVEL)

#define MEMORIA_EXTERN_BASIC_CONTAINER(CollectionName,  CtrName) MEMORIA_EXTERN_CONTAINER(CollectionName, CtrName, \
        v1::btree::BTreeCtrBase, v1::BTreeIteratorBase)






#define MEMORIA_EXTERN_CTR_PAPRT(CollectionName,  CtrName, PartName)                            \
    MEMORIA_TEMPLATE_EXTERN template class CtrPart<PartName, CtrHelper<IndexOfTool<PartName,    \
    CollectionName::Types<CtrName>::Type::CtrTypes::CtrList>::Value - 1,                        \
    CollectionName::Types<CtrName>::Type::CtrTypes>, CollectionName::Types<CtrName>::Type::CtrTypes>;

#define MEMORIA_EXTERN_DATAPAGE(CollectionName,  CtrName, PageType, Level)                          \
    MEMORIA_TEMPLATE_EXTERN template class                                                          \
    PackedSumTree<NodePage<CollectionName::Factory0<CtrName>::Type::PageType<Level> >::MapTypes>;   \
    MEMORIA_TEMPLATE_EXTERN template class                                                          \
    PackedTree<NodePage<CollectionName::Factory0<CtrName>::Type::PageType<Level> >::MapTypes>;


#define MEMORIA_EXTERN_ITER_PAPRT(CollectionName,  CtrName, PartName)                           \
    MEMORIA_TEMPLATE_EXTERN template class IterPart<PartName, IterHelper<IndexOfTool<PartName,  \
    CollectionName::Types<CtrName>::Type::CtrTypes::IterList>::Value - 1,                       \
    CollectionName::Types<CtrName>::Type::CtrTypes::IterTypes>,                                 \
    CollectionName::Types<CtrName>::Type::CtrTypes::IterTypes>;






#define MEMORIA_PAGE_PART_BEGIN(PartName)                                               \
template <typename BaseType>                                                            \
class PagePart<PartName, BaseType>: public BaseType {                                   \
public:                                                                                 \
    typedef BaseType                                                            Base;   \
    typedef PagePart<                                                                   \
                PartName,                                                               \
                BaseType                                                                \
    >                                                                           Me;



#define MEMORIA_PAGE_PART_BEGIN1(PartName, Param)                                       \
template <typename Param, typename BaseType>                                            \
class PagePart<PartName<Param>, BaseType>: public BaseType {                            \
public:                                                                                 \
    typedef BaseType                                                            Base;   \
    typedef PagePart<                                                                   \
                PartName< Param >,                                                      \
                BaseType                                                                \
    >                                                                           Me;




#define MEMORIA_PAGE_PART_BEGIN2(PartName, Param)                                       \
template <Param, typename BaseType>                                                     \
class PagePart<PartName, BaseType>: public BaseType {                                   \
public:                                                                                 \
    typedef BaseType                                                            Base;   \
    typedef PagePart<                                                                   \
                PartName,                                                               \
                BaseType                                                                \
    >                                                                           Me;


#define MEMORIA_PAGE_PART_END };



#define MEMORIA_CONTAINER_PART_NO_CTR_BEGIN(PartName)                           \
template <typename Base1, typename TypesType>                                   \
class CtrPart<PartName, Base1, TypesType>: public Base1 {                       \
    typedef Base1 Base;                                                         \
    typedef CtrPart<PartName, Base1, TypesType> ThisType;                       \
    typedef Ctr<TypesType> MyType;                                              \
    template <typename, typename, typename> friend class CtrPart;               \
    template <typename, typename, typename> friend class IterPart;              \
protected:


#define MEMORIA_CONTAINER_PART_BEGIN(PartName)                                  \
    MEMORIA_CONTAINER_PART_NO_CTR_BEGIN(PartName)                               \
public:                                                                         \
    CtrPart(const CtrInitData& data): Base(data)  {}                            \
    virtual ~CtrPart() throw() {}                                               \
protected:


#define MEMORIA_CONTAINER_PART_END                                              \
    private:                                                                    \
    MyType& self() {                                                            \
        return *static_cast<MyType*>(this);                                     \
    }                                                                           \
                                                                                \
    const MyType& self() const {                                                \
        return *static_cast<const MyType*>(this);                               \
    }                                                                           \
};






#define MEMORIA_CONTAINER_TYPE(PartName)                                        \
CtrPart<PartName, Base, Types>

#define MEMORIA_CONTAINER_TEMPLATE_PARAMS                                       \
template <typename Base, typename Types>



#define MEMORIA_ITERATOR_PART_NO_CTOR_BEGIN(PartName)                           \
template <typename Base1, typename Types>                                       \
class IterPart<PartName, Base1, Types>: public Base1 {                          \
    typedef IterPart<PartName, Base1, Types> ThisPartType;                      \
    typedef Base1 Base;                                                         \
    typedef Iter<Types> MyType;                                                 \
                                                                                \
    template <typename, typename, typename> friend class CtrPart;               \
    template <typename, typename, typename> friend class IterPart;              \
    template <typename> friend class BTIteratorBase;                            \
protected:



#define MEMORIA_ITERATOR_PART_BEGIN(PartName)                                   \
    MEMORIA_ITERATOR_PART_NO_CTOR_BEGIN(PartName)                               \
public:                                                                         \
    IterPart(): Base() {}                                                       \
    IterPart(ThisPartType&& other): Base(std::move(other)) {}                   \
    IterPart(const ThisPartType& other): Base(other) {}                         \
    virtual ~IterPart() throw() {}                                              \
protected:




#define MEMORIA_ITERATOR_PART_END                                               \
    MyType& self() {                                                            \
        return *static_cast<MyType*>(this);                                     \
    }                                                                           \
                                                                                \
    const MyType& self() const {                                                \
        return *static_cast<const MyType*>(this);                               \
    }                                                                           \
};


#define MEMORIA_ITERATOR_TYPE(PartName)                                         \
IterPart<PartName, Base, Types>

#define MEMORIA_ITERATOR_TEMPLATE_PARAMS                                        \
template <typename Base, typename Types>

