#include "Types.h"

#include <sstream>
#include <unordered_set>

std::string TypeInstance::GetTypeName() const 
{ 
    return type_ != nullptr ? type_->typeName : "void"; 
}

bool TypeInstance::Is(ReflectedType* t) const 
{ 
    return t == type_; 
}

bool TypeInstance::Is(AccessModifiers a) const
{
    return accessModifiers_ & a;
}

std::string TypeInstance::ToString(bool withModifiers) const
{
    std::stringstream ss;

    if (withModifiers)
    {
        if (Is(AM_Const))
            ss << "const ";
    }

    ss << type_->typeName;

    if (templateParameters_.size())
    {
        ss << "<";
        for (int i = 0; i < templateParameters_.size(); ++i)
        {
            if (i > 0)
                ss << ", ";
            if (templateParameters_[i].second == INT_MAX)
                ss << templateParameters_[i].first.ToString(true); // always true for this or it'd change the underlying template instantiation
            else
                ss << "int";
        }
        ss << ">";
    }

    if (withModifiers)
    {
        if (Is(AM_Pointer))
            ss << "*";
        if (Is(AM_Reference))
            ss << "&";
    }

    return ss.str();
}

std::string Property::GetFullTypeName() const {
    if (typeHandle_.type_ == nullptr)
        return "void";

    auto type = typeHandle_.type_;
    std::string ret = type->typeName;
    if (type->isTemplate && typeHandle_.templateParameters_.size())
    {
        ret += "<";
        for (unsigned i = 0; i < typeHandle_.templateParameters_.size(); ++i)
        {
            if (typeHandle_.templateParameters_[i].second != INT_MAX)
                ret += "int";
            else
                ret += typeHandle_.templateParameters_[i].first.type_->typeName;
            if (i > 0)
                ret += ", ";
        }
        ret += ">";
    }
    if (Is(AM_Pointer))
        ret += "*";
    if (Is(AM_Reference))
        ret += "&";
    return ret;
}

unsigned ReflectionDatabase::ResolveIncompleteTypes()
{
    unsigned ct = 0;
    for (auto type : types_)
    {
        for (auto property : type.second->properties_)
            ResolveIncompleteType(property->typeHandle_, ct);

        for (auto meth : type.second->methods_)
        {
            ResolveIncompleteType(meth->returnTypeHandle_, ct);
            for (auto p : meth->argumentTypes_)
                ResolveIncompleteType(p->typeHandle_, ct);
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

    for (auto g : globalProperties_)
        ResolveIncompleteType(g->typeHandle_, ct);

    for (auto m : globalFunctions_)
    {
        ResolveIncompleteType(m->returnTypeHandle_, ct);
        for (auto p : m->argumentTypes_)
            ResolveIncompleteType(p->typeHandle_, ct);
    }

    return ct;
}

void ReflectionDatabase::ResolveIncompleteType(TypeInstance& t, unsigned& ct)
{
    if (!t.type_->isComplete)
    {
        auto orig = t.type_;
        if (auto found = GetType(t.type_->typeName))
        {
            t.type_ = found;
            ++ct;
            delete orig;
        }
    }

    for (auto tt : t.templateParameters_)
    {
        if (tt.second == INT_MAX && !tt.first.type_->isComplete)
            ResolveIncompleteType(tt.first, ct);
    }
}

void CountIncomplete(TypeInstance& t, std::unordered_set<std::string>& list)
{
    if (!t.type_->isComplete)
        list.insert(t.type_->typeName);
    for (auto tt : t.templateParameters_)
    {
        if (tt.second == INT_MAX)
            CountIncomplete(tt.first, list);
    }
}

unsigned ReflectionDatabase::CountIncompleteTypes() const 
{
    std::unordered_set<std::string> list;
    for (auto g : globalProperties_)
        CountIncomplete(g->typeHandle_, list);

    for (auto m : globalFunctions_)
    {
        CountIncomplete(m->returnTypeHandle_, list);
        for (auto tt : m->argumentTypes_)
            CountIncomplete(tt->typeHandle_, list);
    }

    for (auto t : types_)
    {
        for (auto p : t.second->properties_)
            CountIncomplete(p->typeHandle_, list);
        for (auto m : t.second->methods_)
        {
            CountIncomplete(m->returnTypeHandle_, list);
            for (auto arg : m->argumentTypes_)
                CountIncomplete(arg->typeHandle_, list);
        }
    }

    return list.size();
}
