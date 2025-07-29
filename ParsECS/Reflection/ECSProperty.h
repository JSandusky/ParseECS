#pragma once

#include <cstdint>
#include <string>

struct ECSPropertyAccessor
{
    virtual void Get(void* object) const = 0;
    virtual void Set(void* object, int value) const = 0;
    virtual bool CanReset() const { return false; }
    virtual void Reset(void* object) const = 0;
};

/// Accesses a property by value
template<typename TYPE, typename INTERNAL_TYPE>
struct MemoryPropertyAccessor : public ECSPropertyAccessor {

    MemoryPropertyAccessor(size_t offset, TYPE defaultVal) : offset_(offset), defaultValue_(defaultVal) { }

    virtual void Get(void* obj) const override {
        unsigned char* src = reinterpret_cast<unsigned char*>(obj);
        // return (TYPE)*((INTERNAL_TYPE*)(src + offset_));
    }

    virtual void Set(void* obj, int value) const override {
        unsigned char* src = reinterpret_cast<unsigned char*>(obj);
        // *((INTERNAL_TYPE*)(src + offset_)) = ((INTERNAL_TYPE)variant.get<TYPE>());
    }

    virtual bool CanReset() const override { return true; }
    
    virtual void Reset(void* object) const override { return Set(object, defaultValue_); }

    size_t offset_;
    TYPE defaultValue_;
};

struct ECSProperty
{
    std::string name_;
    std::string description_;
    const char** enumNames_ = 0;
    uint32_t flags_ = 0;
    const ECSPropertyAccessor* accessor_ = 0x0;

    ECSProperty(const std::string& name, const std::string& description, const ECSPropertyAccessor* accessor, uint32_t flags = 0, const char** enumNames = 0x0) :
        name_(name),
        description_(description),
        accessor_(accessor),
        flags_(flags),
        enumNames_(enumNames)
    {

    }

    void Get(void* object) { return accessor_->Get(object); }
    void Set(void* object, int value) { accessor_->Set(object, value); }
};