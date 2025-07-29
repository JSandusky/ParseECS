#pragma once

#include <cstdint>
#include <unordered_map>

struct StringHash
{
    uint32_t value_ = -1;

    StringHash() { }
    StringHash(const char* str) { }

    bool operator==(const StringHash& rhs) const { return rhs.value_ == value_; }
    bool operator!=(const StringHash& rhs) const { return rhs.value_ != value_; }
    bool operator<(const StringHash& rhs) const { return rhs.value_ < value_; }

    operator uint32_t() { return value_; }
};

namespace std {

    /// Support for std::unordered_map and other containers using hash.
    template<>
    struct hash<StringHash> {
        std::size_t operator()(const StringHash& rhs) { return (size_t)rhs.value_; }
    };
}