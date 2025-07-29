#include "Types.h"

std::string Property::GetFullTypeName() const {
    std::string ret = type_->typeName;
    if (type_->isTemplate && templateParameters_.size())
    {
        ret += "<";
        for (unsigned i = 0; i < templateParameters_.size(); ++i)
        {
            ret += templateParameters_[i]->typeName;
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