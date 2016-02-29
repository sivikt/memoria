
// Copyright Victor Smirnov 2011.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)



#ifndef _MEMORIA_CORE_CONTAINER_MODEL_HPP
#define _MEMORIA_CORE_CONTAINER_MODEL_HPP

#include <memoria/core/types/types.hpp>

#include <memoria/core/types/typelist.hpp>
#include <memoria/core/tools/reflection.hpp>
#include <memoria/core/tools/assert.hpp>
#include <memoria/core/tools/uuid.hpp>

#include <memoria/metadata/container.hpp>

#include <memoria/core/container/logs.hpp>
#include <memoria/core/container/names.hpp>
#include <memoria/core/container/builder.hpp>
#include <memoria/core/container/iterator.hpp>
#include <memoria/core/container/dispatcher.hpp>
#include <memoria/core/container/defaults.hpp>
#include <memoria/core/container/macros.hpp>
#include <memoria/core/container/init.hpp>

#include <string>
#include <memory>



#define MEMORIA_MODEL_METHOD_IS_NOT_IMPLEMENTED() \
        throw Exception(MEMORIA_SOURCE, SBuf()<<"Method is not implemented for "<<me()->typeName())

namespace memoria    {

template <typename Profile, typename SelectorType, typename ContainerTypeName> class CtrTF;

template <typename Name, typename Base, typename Types> class CtrPart;

template <typename Types> class Ctr;
template <typename Types> class Iter;

template <typename Profile> class MetadataRepository;

constexpr UUID CTR_DEFAULT_NAME = UUID(-1ull, -1ull);

class CtrInitData {
    UUID master_name_;
    Int master_ctr_type_hash_;
    Int owner_ctr_type_hash_;

public:
    CtrInitData(const UUID& master_name, Int master_hash, Int owner_hash):
        master_name_(master_name),
        master_ctr_type_hash_(master_hash),
        owner_ctr_type_hash_(owner_hash)
    {}

    CtrInitData(Int master_hash):
        master_name_(),
        master_ctr_type_hash_(master_hash),
        owner_ctr_type_hash_()
    {}

    const auto& master_name() const {
        return master_name_;
    }

    void set_master_name(UUID name){
        master_name_ = name;
    }

    Int owner_ctr_type_hash() const {
        return owner_ctr_type_hash_;
    }

    Int master_ctr_type_hash() const {
        return master_ctr_type_hash_;
    }

    CtrInitData owner(int owner_hash) const
    {
        return CtrInitData(master_name_, master_ctr_type_hash_, owner_hash);
    }
};


template <typename TypesType>
class CtrBase: public TypesType::Allocator, public std::enable_shared_from_this<Ctr<TypesType>> {
public:

    using ThisType  = CtrBase<TypesType>;
    using MyType	= Ctr<TypesType>;

    using ContainerTypeName = typename TypesType::ContainerTypeName;
    using Name 				= ContainerTypeName;
    using Types 			= TypesType;

    using Allocator = typename Types::Allocator;
    using ID 		= typename Allocator::ID;
    using Page 		= typename Allocator::Page;
    using PageG		= typename Allocator::PageG;

    using Iterator		= Iter<typename Types::IterTypes>;
    using IteratorPtr	= std::shared_ptr<Iterator>;
    
    static const Int CONTAINER_HASH = TypeHash<Name>::Value;

    template <typename> friend class BTIteratorBase;

    template <typename> friend class Iter;
    template <typename, typename, typename> friend class IterPart;
    template <typename> friend class IterStart;

    template <typename> friend class CtrStart;
    template <typename, typename, typename> friend class CtrPart;
    template <typename> friend class Ctr;


protected:
    static ContainerMetadata* reflection_;

    ID root_;

    CtrInitData init_data_;

public:
    CtrBase(const CtrInitData& data): init_data_(data)
    {}


    virtual ~CtrBase() throw () {}

    void set_root(const ID &root)
    {
    	root_ = root;

    	self().allocator().setRoot(self().master_name(), root);
    }

    void set_root_id(const ID &root)
    {
    	root_ = root;
    }

    const ID &root() const
    {
        return root_;
    }

    void operator=(ThisType&& other)
    {
        init_data_  = other.init_data_;

        other.shared_ = NULL;
    }

    void operator=(const ThisType& other)
    {
        init_data_  = other.init_data_;
    }

    static Int hash() {
        return CONTAINER_HASH;
    }

    static ContainerMetadata* getMetadata()
    {
        return reflection_;
    }
    
    static void destroyMetadata()
    {
        if (reflection_ != nullptr)
        {
            delete reflection_->getCtrInterface();

            MetadataRepository<typename Types::Profile>::unregisterMetadata(reflection_);

            delete reflection_;
            reflection_ = nullptr;
        }
    }


    struct CtrInterfaceImpl: public ContainerInterface {

        virtual bool check(const void* id, const UUID& name, void* allocator) const
        {
            Allocator* alloc = T2T<Allocator*>(allocator);
            ID* root_id = T2T<ID*>(id);

            PageG page = alloc->getPage(*root_id, name);

            MyType ctr(alloc, *root_id, CtrInitData(name, page->master_ctr_type_hash(), page->owner_ctr_type_hash()));
            return ctr.check(nullptr);
        }

        virtual void walk(
                const void* id,
                const UUID& name,
                void* allocator,
                ContainerWalker* walker
        ) const
        {
            Allocator* alloc = T2T<Allocator*>(allocator);
            ID* root_id = T2T<ID*>(id);

            PageG page = alloc->getPage(*root_id, name);

            MyType ctr(alloc, *root_id, CtrInitData(name, page->master_ctr_type_hash(), page->owner_ctr_type_hash()));

            ctr.walkTree(walker);
        }

        virtual void drop(const UUID& root_id, const UUID& name, void* allocator)
        {
            Allocator* alloc = T2T<Allocator*>(allocator);

            PageG page = alloc->getPage(root_id, name);

            MyType ctr(alloc, root_id, CtrInitData(name, page->master_ctr_type_hash(), page->owner_ctr_type_hash()));
            return ctr.drop();
        }

        virtual String ctr_type_name() const
        {
            return TypeNameFactory<ContainerTypeName>::name();
        }

        virtual ~CtrInterfaceImpl() {}
    };


    static ContainerInterface* getContainerInterface()
    {
        return new CtrInterfaceImpl();
    }

    static Int initMetadata(Int salt = 0)
    {
        if (reflection_ == nullptr)
        {
            MetadataList list;

            Types::Pages::NodeDispatcher::buildMetadataList(list);

            reflection_ = new ContainerMetadata(TypeNameFactory<Name>::name(),
                                                list,
                                                CONTAINER_HASH,
                                                MyType::getContainerInterface());

            MetadataRepository<typename Types::Profile>::registerMetadata(reflection_);
        }

        return reflection_->ctr_hash();
    }

    const CtrInitData& init_data() const {
        return init_data_;
    }

    CtrInitData& init_data() {
        return init_data_;
    }


    UUID getModelName(ID root_id)
    {
        return UUID();
    }


    void initCtr(Int command) {}
    void initCtr(const ID& root_id) {}

protected:

    template <typename... Args>
    IteratorPtr make_iterator(Args&&... args) const {
    	return make_shared<Iterator>(this->shared_from_this(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    IteratorPtr make_iterator(Args&&... args) {
    	return make_shared<Iterator>(this->shared_from_this(), std::forward<Args>(args)...);
    }

    template <typename... Args>
    IteratorPtr clone_iterator(Args&&... args) const {
    	return make_shared<Iterator>(std::forward<Args>(args)...);
    }

    template <typename... Args>
    IteratorPtr clone_iterator(Args&&... args) {
    	return make_shared<Iterator>(std::forward<Args>(args)...);
    }


    /**
     * \brief Set container reflection metadata.
     */

    static void setMetadata(ContainerMetadata* metadata)
    {
        reflection_ = metadata;
    }


private:


    MyType& self()
    {
        return *static_cast<MyType*>(this);
    }

    const MyType& self() const
    {
        return *static_cast<const MyType*>(this);
    }
};

template <typename TypesType>
ContainerMetadata* CtrBase<TypesType>::reflection_ = NULL;



template <int Idx, typename Types>
class CtrHelper: public CtrPart<
                            SelectByIndex<Idx, typename Types::List>,
                            CtrHelper<Idx - 1, Types>,
                            Types>
{
    typedef CtrHelper<Idx, Types>                               ThisType;
    typedef Ctr<Types>                                          MyType;
    typedef CtrPart<SelectByIndex<Idx, typename Types::List>, CtrHelper<Idx - 1, Types>, Types> Base;

    typedef typename Types::Allocator Allocator0;

public:
    CtrHelper(const CtrInitData& data): Base(data) {}

    virtual ~CtrHelper() throw () {}
};

template <typename Types>
class CtrHelper<-1, Types>: public Types::template BaseFactory<Types>::Type {
    typedef CtrHelper<-1, Types>                                ThisType;
    typedef Ctr<Types>                                          MyType;
    typedef typename Types::template BaseFactory<Types>::Type   Base;

public:

    typedef typename Types::Allocator                           Allocator0;

    CtrHelper(const CtrInitData& data): Base(data) {}

    virtual ~CtrHelper() throw () {}

    void operator=(ThisType&& other) {
        Base::operator=(std::move(other));
    }

    void operator=(const ThisType& other) {
        Base::operator=(other);
    }
};


template <typename Types>
class CtrStart: public CtrHelper<ListSize<typename Types::List>::Value - 1, Types> {

    typedef CtrStart<Types>         ThisType;
    typedef Ctr<Types>              MyType;

    typedef CtrHelper<ListSize<typename Types::List>::Value - 1, Types> Base;

    typedef typename Types::Allocator                                   Allocator0;

public:
    CtrStart(const CtrInitData& data): Base(data) {}
};


extern Int CtrRefCounters;
extern Int CtrUnrefCounters;


template <typename Types>
class Ctr: public CtrStart<Types> {
    typedef CtrStart<Types>                                                     Base;
public:
    typedef Ctr<Types>                                                          MyType;

    typedef typename Types::Allocator                                           Allocator;
    typedef typename Types::Allocator::PageG                                    PageG;
    typedef typename PageG::Page::ID                                            ID;

public:

    typedef typename Types::ContainerTypeName                                   ContainerTypeName;
    typedef ContainerTypeName                                                   Name;

private:

    Allocator*  allocator_;
    UUID      	name_;
    const char* model_type_name_;

    Logger          logger_;
    static Logger   class_logger_;

    bool        debug_;

    Int         owner_ctr_type_hash_ = 0;
    Int         master_ctr_type_hash_ = 0;

public:

    Ctr(
            Allocator* allocator,
            Int command = CTR_CREATE,
            const UUID& name = CTR_DEFAULT_NAME,
            const char* mname = NULL
    ):
        Base(CtrInitData(Base::CONTAINER_HASH)),
        allocator_(allocator),
        model_type_name_(mname != NULL ? mname : TypeNameFactory<ContainerTypeName>::cname()),
        debug_(false)
    {
        MEMORIA_ASSERT_NOT_NULL(allocator);

        checkCommandArguments(command, name);

        initLogger();

        if (name == CTR_DEFAULT_NAME)
        {
            initCtr(allocator, allocator->createCtrName(), command, model_type_name_);
        }
        else {
            initCtr(allocator, name, command, model_type_name_);
        }
    }

    void checkCommandArguments(Int command, const UUID& name)
    {
        if ((command & CTR_CREATE) == 0 && (command & CTR_FIND) == 0)
        {
            throw memoria::Exception(MEMORIA_SOURCE, "Either CTR_CREATE, CTR_FIND or both must be specified");
        }

        if ((command & CTR_FIND) && name == CTR_DEFAULT_NAME)
        {
            throw memoria::Exception(MEMORIA_SOURCE, "Container name must be specified for the CTR_FIND operation");
        }
    }


    Ctr(Allocator* allocator, const ID& root_id, const CtrInitData& ctr_init_data, const char* mname = NULL):
        Base(ctr_init_data),
        allocator_(allocator),
        name_(),
        model_type_name_(mname != NULL ? mname : TypeNameFactory<ContainerTypeName>::cname()),
        debug_(false)
    {
        MEMORIA_ASSERT_NOT_NULL(allocator);

        initLogger();

        initCtr(allocator, root_id, mname);
    }

    Ctr(const CtrInitData& data):
        Base(data),
        allocator_(),
        model_type_name_(TypeNameFactory<ContainerTypeName>::cname()),
        logger_(model_type_name_, Logger::DERIVED, NULL),
        debug_(false)
    {
    }

    Ctr(const MyType& other) = delete;
    Ctr(MyType&& other) = delete;

    virtual ~Ctr() throw()
    {}

    void initLogger(Logger* other)
    {
        logger_.setCategory(other->category());
        logger_.setHandler(other->getHandler());
        logger_.setParent(other->getParent());

        logger_.level() = other->level();
    }

    void initLogger()
    {
        logger_.configure(model_type_name_, Logger::DERIVED, &allocator_->logger());
    }

    void initCtr(Allocator* allocator, const UUID& name, Int command, const char* mname = NULL)
    {
        allocator_          = allocator;
        name_               = name;
        model_type_name_    = mname != NULL ? mname : TypeNameFactory<ContainerTypeName>::cname();

        this->init_data().set_master_name(name);

        //FIXME: init logger correctly

        Base::initCtr(command);
    }

    void initCtr(Allocator* allocator, const ID& root_id, const char* mname = NULL)
    {
        MEMORIA_ASSERT_EXPR(!root_id.is_null(), "Container root ID must not be empty");

        allocator_          = allocator;
        model_type_name_    = mname != NULL ? mname : TypeNameFactory<ContainerTypeName>::cname();
        name_               = this->getModelName(root_id);

        //FIXME: init logger correctly

        Base::initCtr(root_id, name_);
    }

    Int owner_ctr_type_hash () const {
        return owner_ctr_type_hash_;
    }

    Int master_ctr_type_hash() const {
        return master_ctr_type_hash_;
    }


    bool& debug() {
        return debug_;
    }

    const bool& debug() const {
        return debug_;
    }

    MEMORIA_PUBLIC Allocator& allocator() {
        return *allocator_;
    }

    MEMORIA_PUBLIC Allocator& allocator() const {
        return *allocator_;
    }

    MEMORIA_PUBLIC const char* typeName() const
    {
        return model_type_name_;
    }

    MEMORIA_PUBLIC static String type_name_str()
    {
        return TypeNameFactory<ContainerTypeName>::name();
    }

    MEMORIA_PUBLIC static const char* type_name_cstr()
    {
        return TypeNameFactory<ContainerTypeName>::cname();
    }


    MEMORIA_PUBLIC bool is_log(Int level) const
    {
        return logger_.isLogEnabled(level);
    }

    MEMORIA_PUBLIC const memoria::Logger& logger() const {
        return logger_;
    }

    MEMORIA_PUBLIC memoria::Logger& logger() {
            return logger_;
        }

    static memoria::Logger& class_logger() {
        return class_logger_;
    }

    const auto& name() const {
        return name_;
    }

    const auto& master_name() const
    {
        return Base::init_data().master_name();
    }

    MyType& operator=(const MyType& other)
    {
        if (this != &other)
        {
            name_               = other.name_;
            model_type_name_    = other.model_type_name_;
            logger_             = other.logger_;
            debug_              = other.debug_;

            Base::operator=(other);
        }

        return *this;
    }

    MyType& operator=(MyType&& other)
    {
        if (this != &other)
        {
            name_               = other.name_;
            model_type_name_    = other.model_type_name_;
            logger_             = other.logger_;
            debug_              = other.debug_;

            Base::operator=(std::move(other));
        }

        return *this;
    }
};

template<
        typename Types
>
Logger Ctr<Types>::class_logger_(typeid(typename Types::ContainerTypeName).name(), Logger::DERIVED, &memoria::logger);


}




#endif
