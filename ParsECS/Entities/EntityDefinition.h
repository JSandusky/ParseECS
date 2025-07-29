#pragma once

#include "../ParsecDef.h"
#include "../Aspect.h"

#include <vector>

struct ComponentBase;

enum EntityDefinitionFlags
{
    EDF_None = 0,
    EDF_Sealed = 1, // Entity is and finalized now
};

struct EntityDefinition
{
    EntityDefinition();
    ~EntityDefinition();

    /// Index in the entity-def table
    uint32_t id_;
    /// 
    uint32_t tag_;
    /// Size of the all component states
    uint32_t stateSize_;
    uint32_t flags_;
    char name_[64];
    ComponentBits mask_;

// Component Retrieval
    // Components are implicitly determined by the mask and id
    ComponentBase* GetComponent(CompID bitIndex);
    ComponentBase* GetComponent(const char* typeName);

    template<typename T>
    T* GetComponent() { return (T*)GetComponent(index); }

// Component Addition
    ComponentBase* AddComponent(CompID compID);
    ComponentBase* AddComponent(const char* typeName);

    template<typename T>
    T* AddComponent() { return AddComponent(T::TypeID); }

// Component removal
    void RemoveComponent(CompID compID);
    void RemoveComponent(const char* typeName);

    template<typename T>
    void RemoveComponent() { RemoveComponent(T::TypeID); }

    std::vector<ComponentBase*> components_;
};