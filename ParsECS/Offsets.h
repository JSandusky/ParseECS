#pragma once

#include "ComponentCount.h"

#include <cstdint>
#include <bitset>

static ComponentBits FromBitIndices(const int* bit, int ct)
{
    ComponentBits ret;
    for (int i = 0; i < ct; ++i)
        ret.set(*bit++, true);
    return ret;
}

template<typename...TList>
struct ComponentMask {
    static const size_t BitCount = sizeof...(TList);
    static const ComponentBits BitSet;
    static const int Indices[BitCount];

    static ComponentBits ToBitSet()
    {
        int bits[sizeof...(TList)] = { (TList::TypeID)... };
        return FromBitIndices(bits, BitCount);
    }
};

template<typename...TList>
const ComponentBits ComponentMask<TList...>::BitSet = ComponentMask::ToBitSet();

template<typename...TList>
const int ComponentMask<TList...>::Indices[ComponentMask<TList...>::BitCount] = { (TList::TypeID)... };


template<typename...Args>
struct ExclusiveScan {
    static const size_t offsets[sizeof...(Args)];
    static const size_t sizes[sizeof...(Args)];
    static const uint32_t ids[sizeof...(Args)];
};

template<typename...Args>
struct ScanTypeSize {
    static const size_t sizes[sizeof...(Args)];
};
template<typename...Args>
const size_t ScanTypeSize<Args...>::sizes[sizeof...(Args)] = { sizeof(Args::State)... };

static size_t PrefixSumMask(const size_t* sizes, size_t sizeCt, size_t flatIndex, std::bitset<PARSECS_COMPONENT_COUNT> bits, const uint32_t* ids)
{
    size_t sum = 0;
    size_t forType = ids[flatIndex];
    for (unsigned i = 0; i < bits.size(); ++i)
    {
        if (bits[i])
        {
            if (i == forType)
                return sum;
            for (unsigned t = 0; t < sizeCt; ++t)
            {
                if (ids[t] == i)
                {
                    sum += sizes[t];
                    break;
                }
            }
        }
    }
    return sum;
}

template<typename...Args>
const uint32_t ExclusiveScan<Args...>::ids[sizeof...(Args)] = { (Args::TypeID)... };

template<typename...Args>
const size_t ExclusiveScan<Args...>::sizes[sizeof...(Args)] = { sizeof(Args::State)... };

template<typename...Args>
const size_t ExclusiveScan<Args...>::offsets[sizeof...(Args)] = { PrefixSumMask(ExclusiveScan<Args...>::sizes, sizeof...(Args), FlatIndex(ComponentMask<Args...>::ToBitSet(), Args::TypeID), ComponentMask<Args...>::ToBitSet(), ids)... };

//template<typename...Args>
//const size_t ExclusiveScan<Args...>::offsets[sizeof...(Args)] = { PrefixSum(ExclusiveScan<Args...>::sizes, FlatIndex(ComponentMask<Args...>::ToBitSet(), Args::TypeID), sizeof...(Args))... };