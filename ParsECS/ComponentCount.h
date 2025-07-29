#pragma once

#include <bitset>

#define PARSECS_COMPONENT_COUNT 32

typedef std::bitset<PARSECS_COMPONENT_COUNT> ComponentBits;

inline unsigned FlatIndex(const ComponentBits& referenceBits, size_t index)
{
    unsigned flatIndex = 0;
    for (unsigned i = 0; i < referenceBits.size(); ++i)
    {
        if (referenceBits.test(i) && i != index)
            ++flatIndex;
        else if (i == index)
            return flatIndex;
    }
    return flatIndex;
}

inline size_t PrefixSum(const size_t* sizes, size_t index, size_t count)
{
    size_t accum = 0;
    for (size_t i = 0; i < count && i < index; ++i)
        accum += sizes[i];
    return accum;
}

inline size_t PrefixSum(std::initializer_list<size_t> sizes, size_t index)
{
    size_t accum = 0;
    auto cur = sizes.begin();
    for (size_t i = 0; cur != sizes.end() && i < index; ++i, ++cur)
        accum += *cur;
    return accum;
}

