#pragma once

#include "SysDef.h"

#include <map>
#include <string>
#include <vector>

/// Serializer is potentially a two way serializer. It may be in write mode or in read mode.
/// Usage: serializer->Serialize(valueToSerialize);
class SYS_EXPORT Serializer
{
public:
    virtual size_t Serialize(void* data, size_t size) = 0;
    virtual size_t GetPosition() const = 0;
    virtual size_t Seek(size_t newPosition) = 0;
    virtual bool IsAtEnd() const = 0;
    virtual bool IsInputMode() const = 0;

// Primitive reference/pointer and array (by pointer and count) support
#define PRIMITIVE(TYPE, NAME) virtual bool Serialize(TYPE& value) { return Serialize(&value, sizeof(TYPE)) == sizeof(TYPE); } \
                        virtual bool Serialize(TYPE* value, size_t count = 1) { return Serialize(value, sizeof(TYPE) * count) == sizeof(TYPE) * count; } \
                        virtual TYPE Read ## NAME() { TYPE ret; Serialize(ret); return ret; }
    PRIMITIVE(bool, Bool);
    PRIMITIVE(int8_t, Byte);
    PRIMITIVE(uint8_t, UByte);
    PRIMITIVE(int32_t, Int);
    PRIMITIVE(uint32_t, UInt);
    PRIMITIVE(int64_t, Int64);
    PRIMITIVE(uint64_t, UInt64);
    PRIMITIVE(float, Float);
    PRIMITIVE(double, Double);
#undef PRIMITIVE

    template<typename T>
    bool Serialize(T& value) { return Serialize((void*)&value, sizeof(T)) == sizeof(T); }
    template<typename T>
    bool Serialize(T* value, size_t count = 1) { return Serialize((void*)value, sizeof(T) * count) == sizeof(T) * count; }

    // std::string
    virtual bool Serialize(std::string& value) { return Serialize((void*)value.c_str(), value.length() + 1) == value.length() + 1; }
    virtual bool Serialize(std::string* value, size_t count = 1) { bool ret = true;  for (unsigned i = 0; i < count; ++i) ret &= Serialize(value[i]); return ret; }
    virtual std::string ReadString() { std::string ret; Serialize(ret); return ret; }

    /// Templated form for STL vector of primitives
    template<typename T>
    bool Serialize(std::vector<T>& vector) { 
        unsigned count = vector.size(); 
        bool ret = Serialize(count);
        // works both ways, if writing this will do nothing, if reading then we'll be the size we need to be
        vector.resize(count);
        for (unsigned i = 0; i < count; ++i)
            ret &= Serialize(vector[i]);
        return ret;
    }

    /// Template utility for writing STL maps of primitive types.
    template<typename K, typename V>
    bool Serialize(std::map<K, V>& map)
    {
        bool ret = true;
        unsigned count = map.size();
        ret &= Serialize(count);

        // Need to handle things differently based on mode since STL map is a sorted container
        if (IsInputMode())
        {
            for (unsigned i = 0; i < count; ++i)
            {
                K key; V value;
                ret &= Serialize(key);
                ret &= Serialize(value);
                map.insert(key, value);
            }
        }
        else
        {
            for (auto rec : map)
            {
                ret &= Serialize(rec.first);
                ret &= Serialize(rec.second);
            }
        }
        return ret;
    }
};

/// A serializer that works with a buffer of bytes via points. May read or write.
SYS_EXPORT class BufferSerializer : public Serializer
{
public:
    BufferSerializer(void* buffer, size_t bufferSize, bool writeMode = false);

// Implement Serializer

    virtual size_t Serialize(void* data, size_t size) override;
    virtual size_t GetPosition() const { return bufferPos_; }
    virtual size_t Seek(size_t newPosition) override;
    virtual bool IsAtEnd() const override { return bufferPos_ == bufferSize_; }
    virtual bool IsInputMode() const override { return writeMode_; }

    void SetWriteMode(bool doWrite) { writeMode_ = doWrite; }

private:
    void* buffer_ = 0;
    size_t bufferSize_ = 0;
    size_t bufferPos_ = 0;
    bool writeMode_ = false;
};

/// A serializer that works with a std::vector. Since it is vector based it can grow to accomodate unknown lengths of data being written.
class SYS_EXPORT VectorSerializer : public Serializer
{
public:
    /// Construct. Will be initialized as write-mode = true
    VectorSerializer();
    /// Construct from an existing byte buffer.
    VectorSerializer(const std::vector<unsigned char>& buffer, bool writeMode = false);
    /// Construct and copy from some input data.
    VectorSerializer(void* buffer, size_t bufferSize, bool writeMode = false);

    // Implement Serializer

    virtual size_t Serialize(void* data, size_t size) override;
    virtual size_t GetPosition() const { return bufferPos_; }
    virtual size_t Seek(size_t newPosition) override;
    virtual bool IsAtEnd() const override { return bufferPos_ == buffer_.size(); }
    virtual bool IsInputMode() const override { return writeMode_; }

    void SetWriteMode(bool doWrite) { writeMode_ = doWrite; }

private:
    std::vector<unsigned char> buffer_;
    size_t bufferPos_ = 0;
    bool writeMode_ = false;
};