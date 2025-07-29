#pragma once

#include <vector>
#include <map>
#include <algorithm>

struct ReflectedType;


enum AccessModifiers
{
    AM_Default = 0,
    AM_Public = 1,
    AM_Protected = 1 << 1,
    AM_Private = 1 << 2,
    AM_Internal = 1 << 3,
    AM_Abstract = 1 << 4,
    AM_Virtual = 1 << 5,
    AM_Override = 1 << 6,
    AM_Const = 1 << 7,
    AM_Pointer = 1 << 8,        // int*, not preferred
    AM_Reference = 1 << 9,      // int&, not preferred
    AM_Template = 1 << 10,      // it's a template type
    AM_Mutable = 1 << 11,
    AM_Volatile = 1 << 12,
    AM_ThreadLocal = 1 << 13,
    AM_Extern = 1 << 14,
};

static bool HasBindingSwitch(const std::vector<std::string>& traits, const std::string& name)
{
    for (unsigned i = 0; i < traits.size(); ++i)
        if (traits[i] == name)
            return true;
    return false;
}

/// Test whether a multivalued binding property is present
static bool HasBindingProperty(const std::vector<std::string>& traits, const std::string& name, unsigned offset = 1)
{
    if (((int)traits.size()) - (int)offset < 0)
        return false;
    for (unsigned i = 0; i < traits.size() - offset; ++i)
        if (traits[i] == name)
            return true;
    return false;
}

/// Get the value of a multivalued binding property
static std::string GetBindingProperty(const std::vector<std::string>& traits, const std::string& name, unsigned offset = 1)
{
    for (unsigned i = 0; i < traits.size() - offset; ++i)
    {
        if (traits[i] == name)
            return traits[i + offset];
    }
    return std::string();
}

struct TypeInstance {
    /// The root type, such as std::vector
    ReflectedType* type_ = nullptr;
    /// Where do flag bits come from?
    ReflectedType* enumSource_ = nullptr;
    /// Modifiers such as const, mutable, volatile, etc
    unsigned accessModifiers_ = 0;
    /// If we're a template type then include the params here.
    std::vector<TypeInstance*> templateParameters_;
    /// Extra binding data.
    std::vector<std::string> bindingData_;
};

/// Property is used for all of the containment information
struct Property
{
    /// Modifiers on the property.
    unsigned accessModifiers_ = 0;
    /// Name of the property.
    std::string propertyName_ = "";
    /// The type for that property.
    ReflectedType* type_ = 0x0;
    /// Where do flags bits come from?
    ReflectedType* enumSource_ = 0x0;
    /// If we're a template property then include the params here.
    std::vector<ReflectedType*> templateParameters_;
    /// Extra binding data for it.
    std::vector<std::string> bindingData_;
    /// Size of the array if this property is an array, will be 0 if not an array.
    unsigned arraySize_ = 0;
    /// Virtual property flag indicating this isn't real but instead backed by functions
    bool isVirtual_ = false;

    /// Returns an approximate C++ fully qualified name
    std::string GetFullTypeName() const;

    /// Check for binding value
    bool HasBindingSwitch(const std::string& t) const { return ::HasBindingSwitch(bindingData_, t); }
    /// Wrapper to check if has key for Key-Value binding info
    bool HasBindingProperty(const std::string& p) const { return ::HasBindingProperty(bindingData_, p); }
    /// Wrapper to common function for Key-Value binding information
    std::string GetBindingProperty(const std::string& p) const { return ::GetBindingProperty(bindingData_, p); }

    /// Check if has an access modifier
    bool Is(AccessModifiers mod) const { return accessModifiers_ & mod; }

    bool IsNot(AccessModifiers mod) const { return (accessModifiers_ & mod) == 0; }

    bool Is(const std::vector<AccessModifiers>& mod) const {
        for (auto m : mod)
            if (!(m & accessModifiers_))
                return false;
        return true;
    }

    bool IsNot(const std::vector<AccessModifiers>& mod) const {
        for (auto m : mod)
            if (m & accessModifiers_)
                return false;
        return true;
    }

    bool IsReadOnly() const { return accessModifiers_ & AM_Const || !HasBindingProperty("set"); }
};

struct Method
{
    /// Return type.
    ReflectedType* returnType_ = 0x0;
    /// Name of the function to call.
    std::string methodName_;
    /// List of all of the argument types.
    std::vector<Property*> argumentTypes_;
    /// Verbatim copies of the default args. Must be variant convertible.
    std::vector<std::string> defaultArguments_;
    /// Names of the arguments.
    std::vector<std::string> argumentNames_;
    /// Extra binding data for it.
    std::vector<std::string> bindingData_;

    /// Wrapper to check for single value named flag
    bool HasBindingSwitch(const std::string& t) const { return ::HasBindingSwitch(bindingData_, t); }
    /// Wrapper to check for existence of Key-Value pair property
    bool HasBindingProperty(const std::string& p) const { return ::HasBindingProperty(bindingData_, p); }
    /// Fetch key-value pair property if it exists, empty string if not
    std::string GetBindingProperty(const std::string& p) const { return ::GetBindingProperty(bindingData_, p); }

    bool IsReturning() const { return returnType_ != nullptr; }
};

struct ReflectedType
{
    /// Has the type been completely resolved?
    bool isComplete = true;
    /// Is this a basic primitive type?
    bool isPrimitive = false;
    /// Is this an array?
    bool isArray = false;
    /// Is this a template type?
    bool isTemplate = false;
    /// If this is an array how long is it?
    unsigned arrayLength = -1;
    /// Includes template factors
    std::string typeName;
    /// Mapped as <access_modifiers, Type>
    std::vector<ReflectedType*> baseClass_;
    /// Assigned state object type
    ReflectedType* stateType_ = 0x0;
    /// If not null then we're a template type.
    ReflectedType* templateParameterType_ = 0x0;
    /// List of the members of the type.
    std::vector<Property*> properties_;
    /// List of the bound methods in the type.
    std::vector<Method*> methods_;
    /// Extra binding data for it.
    std::vector<std::string> bindingData_;
    /// Not-null if we're an enum class
    ReflectedType* enumType_ = nullptr;
    /// If not empty then the type is an enum
    std::vector< std::pair<std::string, int> > enumValues_;
    /// VT_??? for the variant type to use for the object, if empty then we assume to treat as VT_VoidPtr
    std::string variantType_ = "VT_VoidPtr";
    /// Default stringified value for what the variant uses
    std::string variantDefault_ = "0x0";

    ReflectedType() { }
    ReflectedType(std::string primName) : isPrimitive(true), typeName(primName) { }

    /// Isolates the namespace "std::string" -> "std"
    std::string GetNamespace() const {
        auto idx = typeName.find_first_of(':');
        if (idx != std::string::npos)
            return typeName.substr(0, idx - 1);
        return "";
    }

    /// Isolates the typename from the namespace "std::string" -> "string"
    std::string GetUnscopedName() const {
        auto idx = typeName.find_last_of(':');
        if (idx != std::string::npos)
            return typeName.substr(idx + 1);
        return typeName;
    }

    /// Wrapper to check for named flag in binding info
    bool HasBindingSwitch(const std::string& t) const { return ::HasBindingSwitch(bindingData_, t); }
    /// Wrapper to check for Key-Value pair existence
    bool HasBindingProperty(const std::string& p) const { return ::HasBindingProperty(bindingData_, p); }
    /// Get Key-Value pair value, empty string if it doesn't exist
    std::string GetBindingProperty(const std::string& p) const { return ::GetBindingProperty(bindingData_, p); }

    bool Is(const char* name) const { return typeName == name; }
    bool Is(const std::string& name) const { return typeName == name; }
    bool IsEnum() const { return !enumValues_.empty(); }
    bool IsEnumClass() const { return enumType_ != nullptr; }
    bool InheritsFrom(const ReflectedType* t) const {
        if (t == this) // identity check
            return true;

        for (auto bt : t->baseClass_)
        {
            if (bt == t) // is this guy our target?
                return true;
            if (bt->InheritsFrom(t)) // recurse
                return true;
        }
        return false;
    }
    bool BaseOf(ReflectedType* t) const { return t->InheritsFrom(this); }

    std::vector<ReflectedType*> GetInheritanceTreeTo(const ReflectedType* t) const {
        std::vector<ReflectedType*> ret;
        GetInheritanceTreeTo(t, ret);
        return ret;
    }

    bool GetInheritanceTreeTo(const ReflectedType* t, std::vector<ReflectedType*>& holder) const {
        if (t == this)
            return false;

        for (auto bt : t->baseClass_)
        {
            if (bt == t)
            {
                holder.push_back(bt);
                return true;
            }

            holder.push_back(bt);
            if (bt->GetInheritanceTreeTo(t, holder))
            {
                return true;
            }
            else
                holder.pop_back();
        }

        return false;
    }
};

struct ReflectionDatabase
{
    /// Table of all known types
    std::map<std::string, ReflectedType*> types_;
    /// List of global scope functions
    std::vector<Method*> globalFunctions_;
    /// List of global scope properties
    std::vector<Property*> globalProperties_;

    /// Visits all of the types and if they have anything whose types are incomplete will replace them
    /// TODO: fix the zero shits concern
    void ResolveIncompleteTypes()
    {
        for (auto type : types_)
        {
            for (auto property : type.second->properties_)
            {
                if (property->type_ && !property->type_->isComplete)
                {
                    if (auto found = GetType(property->type_->typeName.c_str()))
                    {
                        delete property->type_;
                        property->type_ = found;
                    }
                }
            }

            // class type enums
            if (type.second->enumType_ && !type.second->enumType_->isComplete)
            {
                if (auto found = GetType(type.second->enumType_->typeName))
                {
                    delete type.second->enumType_;
                    type.second->enumType_ = found;
                }
            }
        }
    }

    /// Find type by identifier, often needs to be fully scoped as seen (std::string not string)
    ReflectedType* GetType(const char* identifier) const
    {
        auto found = types_.find(identifier);
        if (found != types_.end())
            return found->second;
        return 0x0;
    }

    /// Find type by identifier, often needs to be fully scoped as seen (std::string not string)
    ReflectedType* GetType(const std::string& identifier) const
    {
        auto found = types_.find(identifier);
        if (found != types_.end())
            return found->second;
        return 0x0;
    }

    /// Returns the evaluated value of named enum member, examines all enum types and finds the match
    int GetPossibleLiteral(const char* identifier)
    {
        for (auto typeInfo : types_)
        {
            if (!typeInfo.second->enumValues_.empty())
            {
                for (unsigned i = 0; i < typeInfo.second->enumValues_.size(); ++i)
                {
                    if (typeInfo.second->enumValues_[i].first == identifier)
                        return typeInfo.second->enumValues_[i].second;
                }
            }
        }
        return 0;
    }

    /// Helper to check if a type is a given type by name
    bool IsType(const ReflectedType* t, const char* name) const { return t == GetType(name); }
};

