
// Copyright 2012 Victor Smirnov
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
#include <memoria/v1/core/types/type2type.hpp>
#include <memoria/v1/core/exceptions/exceptions.hpp>
#include <memoria/v1/core/tools/strings/string.hpp>
#include <memoria/v1/core/tools/static_array.hpp>
#include <memoria/v1/tools/configuration.hpp>

#include <limits>
#include <stdlib.h>
#include <iostream>
#include <sstream>


namespace memoria {
namespace v1 {

using namespace std;

class AbstractParamDescriptor {
public:
    virtual void Process(Configurator* cfg)                             = 0;
    virtual StringRef getName() const                                   = 0;
    virtual String getPropertyName() const                              = 0;
    virtual void dump(std::ostream& os, bool dump_prefix = true) const  = 0;
    virtual bool isStateParameter() const                               = 0;
    
    virtual ~AbstractParamDescriptor() {}
};


template <typename T> class ParamDescriptor;

class ParametersSet {

    String          name_;
    ParametersSet*  context_;

    vector<AbstractParamDescriptor*> descriptors_;

public:
    ParametersSet(StringRef name): name_(name), context_(nullptr)  {}
    ParametersSet(const ParametersSet&) = delete;

    virtual ~ParametersSet()
    {
        for (auto d: descriptors_)
        {
            delete d;
        }
    }

    virtual String getFullName() const
    {
        if (context_)
        {
            return context_->getFullName()+"."+name_;
        }
        else {
            return name_;
        }
    }

    virtual StringRef getName() const {
        return name_;
    }

    const ParametersSet* getContext() const
    {
        return context_;
    }

    void setContext(ParametersSet* context)
    {
        this->context_ = context;
    }

    virtual AbstractParamDescriptor* put(AbstractParamDescriptor* descr);

    template <typename T>
    ParamDescriptor<T>* Add(StringRef name, T& property)
    {
        return T2T_S<ParamDescriptor<T>*>(put(new ParamDescriptor<T>(this, name, property)));
    }

    template <typename T>
    ParamDescriptor<T>* Add(StringRef name, T& property, const T& max_value)
    {
        return T2T_S<ParamDescriptor<T>*>(put(new ParamDescriptor<T>(this, name, property, max_value)));
    }

    template <typename T>
    ParamDescriptor<T>* Add(StringRef name, T& property, const T& min_value, const T& max_value)
    {
        return T2T_S<ParamDescriptor<T>*>(put(new ParamDescriptor<T>(this, name, property, min_value, max_value)));
    }

    virtual void dumpProperties(std::ostream& os, bool dump_prefix = true, bool dump_all = false) const;

    void Process(Configurator* cfg);
};


template <typename T>
class ParamDescriptor: public AbstractParamDescriptor {

    typedef ParamDescriptor<T>                      MyType;

    ParametersSet*      cfg_;

    String              name_;
    String              description_;

    T&                  value_;

    T                   min_value_;
    T                   max_value_;

    bool                ranges_specified_;

    bool                mandatory_;

    bool                state_parameter_;

public:

    ParamDescriptor(ParametersSet* cfg, String name, T& value):
        cfg_(cfg),
        name_(name),
        value_(value),
        ranges_specified_(false),
        mandatory_(false),
        state_parameter_(false)
    {}

    ParamDescriptor(ParametersSet* cfg, String name, T& value, const T& max_value):
        cfg_(cfg),
        name_(name),
        value_(value),
        min_value_(numeric_limits<T>::min()),
        max_value_(max_value),
        ranges_specified_(true),
        mandatory_(false),
        state_parameter_(false)
    {}

    ParamDescriptor(ParametersSet* cfg, String name, T& value, const T& min_value, const T& max_value):
        cfg_(cfg),
        name_(name),
        value_(value),
        min_value_(min_value),
        max_value_(max_value),
        ranges_specified_(true),
        mandatory_(false),
        state_parameter_(false)
    {}
    
    virtual ~ParamDescriptor() {}


    virtual void Process(Configurator* cfg)
    {
        setValue(cfg, value_);

        if (ranges_specified_)
        {
            if (!(value_ >= min_value_ && value_ <= max_value_))
            {
                throw Exception(MEMORIA_SOURCE, SBuf()<<"Range checking failure for the property: "<<prefix()<<"."<<name_);
            }
        }
    }

    MyType* setDescription(StringRef descr)
    {
        description_ = descr;
        return this;
    }

    virtual bool IsMandatory() const
    {
        return mandatory_;
    }

    virtual bool isStateParameter() const
    {
        return state_parameter_;
    }

    MyType* setMandatory(bool mandatory)
    {
        mandatory_ = mandatory;
        return this;
    }

    MyType* state(bool state = true)
    {
        state_parameter_ = state;
        return this;
    }

    MyType* minValue(const T& value)
    {
        min_value_ = value;
        return this;
    }

    MyType* maxValue(const T& value)
    {
        max_value_ = value;
        return this;
    }

    virtual StringRef getName() const
    {
        return name_;
    }

    virtual String getPropertyName() const
    {
        if (isEmpty(prefix()))
        {
            return name_;
        }
        else {
            return prefix()+"."+name_;
        }
    }

    virtual void dump(std::ostream& os, bool dump_prefix) const
    {
        if (dump_prefix)
        {
            if (!isEmpty(description_))
            {
                os<<"#"<<description_<<endl;
            }

            if (ranges_specified_)
            {
                os<<"#";
                os<<"Range from: "<<min_value_<<" to "<<max_value_;
                os<<endl;
            }

            os<<getPropertyName()<<" = "<<value_<<endl;

            os<<endl;
        }
        else {
            os<<getName()<<" = "<<value_<<endl;
        }
    }

    String prefix() const
    {
        return cfg_->getFullName();
    }

protected:
    void setValue(Configurator* cfg, T& value)
    {
        StringRef prefix1 = prefix();

        auto pos = prefix1.length();

        while (true)
        {
            String name = prefix().substr(0, pos) + "." + name_;

            if (cfg->IsPropertyDefined(name))
            {
                value = FromString<T>::convert(cfg->getProperty(name));
                return;
            }
            else {
                pos = prefix().find_last_of(".", pos - 1);

                if (pos == String::npos)
                {
                    break;
                }

            }
        }

        if (cfg->IsPropertyDefined(name_))
        {
            value = FromString<T>::convert(cfg->getProperty(name_));
            return;
        }
        else if (mandatory_)
        {
            throw Exception(MEMORIA_SOURCE, SBuf()<<"Property "<<name_<<" is not defined in the program configuration");
        }
    }
};





template <typename T, size_t Size>
class ParamDescriptor<T[Size]>: public AbstractParamDescriptor {

    typedef ParamDescriptor<T[Size]>                                            MyType;

    ParametersSet*      cfg_;

    String              name_;
    String              description_;

    T*                  value_;

    bool                mandatory_;
    bool                state_parameter_;

public:

    ParamDescriptor(ParametersSet* cfg, String name, T* value):
        cfg_(cfg),
        name_(name),
        value_(value),

        mandatory_(false),
        state_parameter_(false)
    {}


    virtual ~ParamDescriptor() {}


    virtual void Process(Configurator* cfg)
    {
        setValue(cfg, value_);
    }

    MyType* setDescription(StringRef descr)
    {
        description_ = descr;
        return this;
    }

    virtual bool IsMandatory() const
    {
        return mandatory_;
    }

    virtual bool isStateParameter() const
    {
        return state_parameter_;
    }

    MyType* setMandatory(bool mandatory)
    {
        mandatory_ = mandatory;
        return this;
    }

    MyType* state(bool state = true)
    {
        state_parameter_ = state;
        return this;
    }

    virtual StringRef getName() const
    {
        return name_;
    }

    virtual String getPropertyName() const
    {
        if (isEmpty(prefix()))
        {
            return name_;
        }
        else {
            return prefix()+"."+name_;
        }
    }

    virtual void dump(std::ostream& os, bool dump_prefix) const
    {
        if (dump_prefix)
        {
            if (!isEmpty(description_))
            {
                os<<"#"<<description_<<endl;
            }

            os<<getPropertyName()<<" = "<<valueToString()<<endl;

            os<<endl;
        }
        else {
            os<<getName()<<" = "<<valueToString()<<endl;
        }
    }

    String prefix() const
    {
        return cfg_->getFullName();
    }

protected:
    void setValue(Configurator* cfg, T* value)
    {
        StringRef prefix1 = prefix();

        auto pos = prefix1.length();

        while (true)
        {
            String name = prefix().substr(0, pos) + "." + name_;

            if (cfg->IsPropertyDefined(name))
            {
                FromString<T[Size]>::convert(value, cfg->getProperty(name));
                return;
            }
            else {
                pos = prefix().find_last_of(".", pos - 1);

                if (pos == String::npos)
                {
                    break;
                }
            }
        }

        if (cfg->IsPropertyDefined(name_))
        {
            FromString<T[Size]>::convert(value, cfg->getProperty(name_));
            return;
        }
        else if (mandatory_)
        {
            throw Exception(MEMORIA_SOURCE, SBuf()<<"Property "<<name_<<" is not defined in the program configuration");
        }
    }

    virtual String valueToString() const
    {
        stringstream str;

        for (size_t c = 0; c < Size; c++)
        {
            str<<value_[c];

            if (c < Size - 1)
            {
                str<<", ";
            }
        }

        return str.str();
    }
};



template <typename T, Int Size>
class ParamDescriptor<core::StaticVector<T, Size>>: public AbstractParamDescriptor {

    using ValueType = core::StaticVector<T, Size>;

    using MyType = ParamDescriptor<ValueType>;



    ParametersSet*      cfg_;

    String              name_;
    String              description_;

    ValueType&          value_;

    bool                mandatory_;
    bool                state_parameter_;

public:

    ParamDescriptor(ParametersSet* cfg, String name, ValueType& value):
        cfg_(cfg),
        name_(name),
        value_(value),

        mandatory_(false),
        state_parameter_(false)
    {}


    virtual ~ParamDescriptor() {}


    virtual void Process(Configurator* cfg)
    {
        setValue(cfg, value_);
    }

    MyType* setDescription(StringRef descr)
    {
        description_ = descr;
        return this;
    }

    virtual bool IsMandatory() const
    {
        return mandatory_;
    }

    virtual bool isStateParameter() const
    {
        return state_parameter_;
    }

    MyType* setMandatory(bool mandatory)
    {
        mandatory_ = mandatory;
        return this;
    }

    MyType* state(bool state = true)
    {
        state_parameter_ = state;
        return this;
    }

    virtual StringRef getName() const
    {
        return name_;
    }

    virtual String getPropertyName() const
    {
        if (isEmpty(prefix()))
        {
            return name_;
        }
        else {
            return prefix()+"."+name_;
        }
    }

    virtual void dump(std::ostream& os, bool dump_prefix) const
    {
        if (dump_prefix)
        {
            if (!isEmpty(description_))
            {
                os<<"#"<<description_<<endl;
            }

            os<<getPropertyName()<<" = "<<valueToString()<<endl;

            os<<endl;
        }
        else {
            os<<getName()<<" = "<<valueToString()<<endl;
        }
    }

    String prefix() const
    {
        return cfg_->getFullName();
    }

protected:
    void setValue(Configurator* cfg, ValueType& value)
    {
        StringRef prefix1 = prefix();

        auto pos = prefix1.length();

        while (true)
        {
            String name = prefix().substr(0, pos) + "." + name_;

            if (cfg->IsPropertyDefined(name))
            {
                FromString<ValueType>::convert(value, cfg->getProperty(name));
                return;
            }
            else {
                pos = prefix().find_last_of(".", pos - 1);

                if (pos == String::npos)
                {
                    break;
                }
            }
        }

        if (cfg->IsPropertyDefined(name_))
        {
            FromString<ValueType>::convert(value, cfg->getProperty(name_));
            return;
        }
        else if (mandatory_)
        {
            throw Exception(MEMORIA_SOURCE, SBuf()<<"Property "<<name_<<" is not defined in the program configuration");
        }
    }

    virtual String valueToString() const
    {
        stringstream str;

        str<<value_;

        return str.str();
    }
};





}}