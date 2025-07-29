#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>

template<typename T>
class MisdirectedVector : public std::vector< std::pair<T, uint32_t> >
{
public:



private:
    std::unordered_map<uint32_t, uint32_t> indirectionTable_;
};