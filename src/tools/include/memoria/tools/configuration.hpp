
// Copyright Victor Smirnov 2012.
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#ifndef MEMORIA_TOOLS_CONFIGURATION_HPP_
#define MEMORIA_TOOLS_CONFIGURATION_HPP_

#include <memoria/core/types/types.hpp>
#include <memoria/core/tools/platform.hpp>
#include <memoria/core/tools/strings.hpp>
#include <memoria/core/exceptions/exceptions.hpp>

#include <map>
#include <set>
#include <vector>

namespace memoria {

using namespace memoria::vapi;

class ListOfStrings;
class Configurator;


class Configurator {
public:

        typedef std::map<String, String>        		StringMapType;
        typedef std::set<String>                        PropertyListType;

        Configurator(Configurator *parent = NULL);
        virtual ~Configurator() throw ();

        virtual Configurator* GetParent() const;
        virtual void SetParent(Configurator* parent);

        virtual void AddProperty(StringRef name, StringRef value);
        virtual void RemoveProperty(StringRef name);

        virtual bool IsPropertyDefined(StringRef name) const;
        virtual String GetProperty(StringRef name, bool resolve = true) const;

        virtual const StringMapType& GetThisConfigurationProperties() const;

        virtual PropertyListType* GetPropertyList() const;

        static Configurator* Parse(StringRef fileName, Configurator* cfg = NULL);

        static Configurator* BuildChain(const char** envp, bool read_config_files = true);

        template <typename T>
        T GetValue(StringRef name)
        {
        	if (this->IsPropertyDefined(name))
        	{
        		return FromString<T>::convert(this->GetProperty(name));
        	}
        	else {
        		throw MemoriaException(MEMORIA_SOURCE, "Property "+name+" is not specified");
        	}
        }

        template <typename T>
        T GetValue(StringRef name, const T& default_value)
        {
        	if (this->IsPropertyDefined(name))
        	{
        		return FromString<T>::convert(this->GetProperty(name));
        	}
        	else
        	{
        		return default_value;
        	}
        }

protected:

        static Configurator* BuildRootConfigurator(const char** envp);
        static Configurator* BuildPlatformDefaultsConfigurator();

        class NameTree {
                const String* name_;
                NameTree* parent_;
        public:
                NameTree(const String* name, NameTree* parent = NULL):
                        name_(name),
                        parent_(parent)
                {}

                bool find(StringRef name) const
                {
                	if (name == *name_)
                	{
                		return true;
                	}
                	else if (parent_ != NULL)
                	{
                		return parent_->find(name);
                	}
                	else
                	{
                		return false;
                	}
                }
        };

        virtual String resolve_references(StringRef value, NameTree* names) const;
        virtual String get_property(StringRef name, NameTree *names, bool resolve) const;

private:
        Configurator* parent_;
        StringMapType properties_;
};

class StringList {
        std::vector<String> list_;
public:
        StringList(StringRef list, StringRef separators = ",");

        int Size() const;
        StringRef GetItem(Int size) const;
};

class PathList: public StringList {
public:
        PathList(StringRef list, StringRef separator = Platform::GetPathSeparator()): StringList(list, separator) {};
};



}


#endif
