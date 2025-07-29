#pragma once

#include "SysDef.h"

#include <cstdint>
#include <string>
#include <vector>

struct ReflectedPropertyAccessor {
    virtual bool IsIndexed() const { return false; }
    /// Gets the value of a member.
    virtual void* Get(void* object) = 0;
    /// Sets the value of a member.
    virtual void Set(void* object, void* value) = 0;
    /// Gets the value at the specified index if an array/list.
    virtual void* GetIndex(void* object, int index) { return 0x0; }
    /// Sets the value at the specified index if an array/list.
    virtual bool SetIndex(void* object, int index, void* value) { };
    /// Implementations should return the current number of objects if an array/list.
    virtual uint32_t GetCount(void* object) { return 0; }
    /// Implementations should set the current number of objects if an array/list.
    virtual uint32_t SetCount(void* object, uint32_t ct) { return 0; }
    /// If the SetCount functions then this function must return true.
    virtual bool CountIsMutable() const { return false; }
    /// Reset the property
    virtual void Reset(void* object) { }
};

// Flags for registered properties, most of these are for GUI use, however the first several include behaviour hints.
enum ReflectedPropertyFlags
{
    RPF_Default = 0,                    // Is a default
    RPF_Secret = 1,                     // Will not be shown in editor GUI (at least not without turning off hiding system properties)
    RPF_DoNotSerialize = 1 << 1,        // Will not be written into files (will still be written into network streams if RPF_Network|RPF_NetworkInterpolate is set)
    RPF_Network = 1 << 2,               // The value is intended for network transmission
    RPF_NetworkInterpolate = 1 << 3,    // The value is both intended for network transmission and should be smoothly interpolated (only supported by float, colors, vectors, and quats)
    RPF_EntityID = 1 << 4,              // The uint32_t object is an entity-ID for an entity
    RPF_ComponentID = 1 << 5,           // The uint32_t object is a component type-ID
    RPF_EntityDefinitionID = 1 << 6,    // The uint32_t object is a type-ID for an entity definition
    RPF_IsIndexed = 1 << 7,             // The property accessor for this is an indexed array
    RPF_IsFixedIndexed = 1 << 8,        // The property accessor is for an indexed array, but the count is fixed
    RPF_EnumAsBitField = 1 << 9,        // Enumeration will be used as a bitfield
    RPF_IntegralAsBitField = 1 << 10,   // An integral type will be used as a bitfield
    RPF_IsActivityControl = 1 << 11,    // Property controls whether the component is 'active,' GUI helper
    RPF_Required = 1 << 12,             // The field can only be set to a valid value
    RPF_ReadOnly = 1 << 13,             // Cannot be edited

// Very specific GUI helpers
    RPF_NoAlpha = 1 << 14,              // Color property will not show alpha channel
    RPF_TinyIncrement = 1 << 15,        // Value will increment by a tiny amount (0.01 per step in GUIs, default is 1.0)
    RPF_SmallIncrement = 1 << 16,       // Value will increment by a small amount (0.1 per step in GUIs, default is 1.0)
    RPF_Translation = 1 << 17,          // Value is intended to receive a translation gizmo (must be Vec2/Vec3/Vec4), only one allowed per object
    RPF_Rotation = 1 << 18,             // Vlaue is intended to receive a rotation gizmo (must be float/Quat), only one allowed per object
    RPF_Scale = 1 << 19,                // Value is intended to receive a scaing gizmo (must be Vec2/Vec3/Vec4), only one allowed per object
    RPF_Transform = 1 << 20,            // Value is intended to receive all gizmos (must be a Mat3x3/Mat3x4), only one allowed per object
    RPF_IsDetail = 1 << 21,             // Property is considered to be 'detail' for any purposes of excluding fields to essential ones
    RPF_IsID = 1 << 22,                 // Property is a unique identifier (will be displayed as part of information names)
    RPF_IsName = 1 << 23,               // Property is a name identifier (will be displayed as part of informative names)
    RPF_IsUNormalRange = 1 << 24,       // Value is expected to be 0 to 1
    RPF_IsSNormalRange = 1 << 25,       // Value is expected to be -1 to 1
    RPF_IsAngularDegRange = 1 << 26,    // Value is expected to be 0 to 360
};

struct ReflectedProperty
{
    /// Name to use for displaying the property.
    std::string propertyName_;
    /// Should contain useful information about the purposes of the property.
    std::string propertyDescription_;
    /// Flags for the property, see the enumeration above.
    uint32_t flags_ = 0;
    /// Names of enums.
    std::vector<std::string> enumNames_; // names of all of the enumerations
    /// Values for each enum, matches each index.
    std::vector<int> enumValues_;
    /// Accessor used for getting to the property.
    ReflectedPropertyAccessor* accessor_;
};

/// This accessor works with byte offsets in memory
template<typename CLASSTYPE, typename T>
struct MemoryPropertyAccessor : public ReflectedPropertyAccessor
{
    unsigned offset_;

    /// Gets the value of a member.
    virtual void* Get(void* object) override
    {
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        T* vector = ((T*)(src + offset_));
        return vector;
    }
    
    /// Sets the value of a member.
    virtual void Set(void* object, void* value) override
    {
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        T* vector = ((T*)(src + offset_));
        *vector = *((T*)value);
    }
};

/// This accessor works with functions, const getter and non-const setter
template<typename CLASSTYPE, typename T>
struct JavaBeanPropertyAccessor : public ReflectedPropertyAccessor
{
    typedef void (CLASSTYPE::*SET_METHOD)(T type);
    typedef T(CLASSTYPE::*GET_METHOD)() const;

    GET_METHOD getter_;
    SET_METHOD setter_;

    virtual void* Get(void* object)
    {
        CLASSTYPE* src = static_cast<CLASSTYPE*>(obj);
        return (src->*getter_)();
    }
    
    virtual void Set(void* object, void* value)
    {
        CLASSTYPE* src = static_cast<CLASSTYPE*>(obj);
        (src->*setter_)(*((T*)value));
    }
};

/// Binds an array (int[32]) to exposure.
template<typename CLASSTYPE, typename T>
struct ArrayReflectedPropertyAccessor : public ReflectedPropertyAccessor
{
    unsigned offset_;
    unsigned length_;

    virtual bool IsIndexed() const override { return true; }
    /// Gets the value of a member.
    virtual void* Get(void* object) { return 0x0; }
    /// Sets the value of a member.
    virtual void Set(void* object, void* value) { }

    virtual void* GetIndex(void* object, int index) {
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        T* vector = ((T*)(src + offset_));
        if (index < vector->size())
            return &(vector[index]);
        return 0x0;
    }

    virtual bool SetIndex(void* object, int index, void* value) {
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        T* vector = ((T*)(src + offset_));
        if (length_ <= index)
            return false;
        vector[index] = *(T*)value;
        return true;
    };

    virtual uint32_t GetCount(void* object) {
        return length_;
    }

    virtual bool CountIsMutable() const { return false; }
};

/// Binds std::vector<T> for access
template<typename CLASSTYPE, typename T>
struct VectorReflectedPropertyAccessor : public ReflectedPropertyAccessor
{
    typedef std::vector<T> vecType;
    unsigned offset_;

    virtual bool IsIndexed() const override { return true; }
    /// Gets the value of a member.
    virtual void* Get(void* object) { return 0x0; }
    /// Sets the value of a member.
    virtual void Set(void* object, void* value) { }
        
    virtual void* GetIndex(void* object, int index) { 
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        vecType* vector = ((vecType*)(src + offset_));
        if (index < vector->size())
            return &(*vector)[index];
        return 0x0;
    }
    
    virtual bool SetIndex(void* object, int index, void* value) { 
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        vecType* vector = ((vecType*)(src + offset_));
        if (vector->size() < index)
            vector->resize(index);
        (*vector)[index] = *(T*)value;
        return true;
    };
    
    virtual uint32_t GetCount(void* object) {
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        vecType* vector = ((vecType*)(src + offset_));
        return vector->size();
    }
    
    virtual uint32_t SetCount(void* object, uint32_t ct) { 
        unsigned char* src = reinterpret_cast<unsigned char*>(object);
        vecType* vector = ((vecType*)(src + offset_));
        vector->resize(ct);
        return vector->size();
    }
    
    
    virtual bool CountIsMutable() const { return true; }
};