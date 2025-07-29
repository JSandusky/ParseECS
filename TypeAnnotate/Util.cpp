#include "Util.h"

TypeInstanceBuilder& TypeInstanceBuilder::MakeConst()
{
    type.accessModifiers_ |= AM_Const;
    return *this;
}
TypeInstanceBuilder& TypeInstanceBuilder::MakePointer()
{
    type.accessModifiers_ |= AM_Pointer;
    return *this;
}
TypeInstanceBuilder& TypeInstanceBuilder::MakeReference()
{
    type.accessModifiers_ |= AM_Reference;
    return *this;
}
TypeInstanceBuilder& TypeInstanceBuilder::MakeType(const char* name, ReflectionDatabase* db)
{
    type.type_ = db->GetType(name);
    return *this;
}
TypeInstanceBuilder& TypeInstanceBuilder::AddTemplateParam(TypeInstance t)
{
    type.templateParameters_.push_back({ t, INT_MAX });

    return *this;
}

bool SignatureMatches(const TypeInstance& lhs, const TypeInstance& rhs)
{
    if (lhs.IsVoid() != rhs.IsVoid())
        return false;
    if (lhs.accessModifiers_ != rhs.accessModifiers_)
        return false;
    if (lhs.type_ != rhs.type_)
        return false;

    if (lhs.templateParameters_.size() != rhs.templateParameters_.size())
        return false;

    for (int i = 0; i < lhs.templateParameters_.size(); ++i)
    {
        if ((lhs.templateParameters_[i].second == INT_MAX) != (rhs.templateParameters_[i].second == INT_MAX))
            return false;
        if (lhs.templateParameters_[i].second != INT_MAX && (lhs.templateParameters_[i].second != rhs.templateParameters_[i].second))
            return false;            
        if (!SignatureMatches(lhs.templateParameters_[i].first, rhs.templateParameters_[i].first))
            return false;
    }

    return true;
}

bool SignatureMatches(Method* meth, const Signature& sig)
{
    if (!SignatureMatches(meth->returnTypeHandle_, sig.returnType_))
        return false;

    if (sig.parameterTypes_.size() != meth->argumentTypes_.size())
        return false;

    for (int i = 0; i < meth->argumentTypes_.size(); ++i)
    {
        if (!SignatureMatches(meth->argumentTypes_[i]->typeHandle_, sig.parameterTypes_[i]))
            return false;
    }

    return true;
}

Signature SignatureOf(Method* meth)
{
    Signature ret;
    ret.returnType_ = meth->returnTypeHandle_;
    for (auto p : meth->argumentTypes_)
        ret.parameterTypes_.push_back(p->typeHandle_);
    return ret;
}