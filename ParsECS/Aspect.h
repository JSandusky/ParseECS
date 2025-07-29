#pragma once

#include "ComponentCount.h"
#include "Offsets.h"

#include <bitset>

struct Aspect
{
    std::bitset<PARSECS_COMPONENT_COUNT> requireMask_;
    std::bitset<PARSECS_COMPONENT_COUNT> excludeMask_;
    std::bitset<PARSECS_COMPONENT_COUNT> oneOf_;

    template<typename...TList>
    Aspect& OneOf()
    {
        oneOf_ = ComponentMask<TList...>::BitSet;
        return *this;
    }

    template<typename...TList>
    Aspect& Require()
    {
        requireMask_ = ComponentMask<TList...>::BitSet;
        return *this;
    }

    template<typename...TList>
    Aspect& Exclude()
    {
        excludeMask_ = ComponentMask<TList...>::BitSet;
        return *this;
    }

    bool Passes(const std::bitset<PARSECS_COMPONENT_COUNT>& mask) const
    {
        if (requireMask_.count() && !(requireMask_ & mask).all())
            return false;
        if (excludeMask_.count() && (excludeMask_ & mask).any())
            return false;
        return (oneOf_ & mask).any();
    }
};