#pragma once

#include "../ComponentCount.h"

#include "Component.h"
#include "ComponentMetaData.h"
#include "../ECSVector.h"
#include "../StrHash.h"

#include <cstdint>
#include <unordered_map>
#include <vector>

struct ComponentBase;
struct ECSProperty;

#define REGISTER_COMPONENT(TYPENAME, STATENAME) ComponentRegistry::Register<TYPENAME, STATENAME>(#TYPENAME, #STATENAME)

/// The global and static data for components is registered into this object
class ComponentRegistry
{
public:
    static inline size_t GetDataSize(uint32_t bitIndex) { return dataSize_[bitIndex]; }

    static inline std::vector< ComponentMetaData* >& GetMetaData() { return metaData_; }

    static ComponentMetaData* GetMetaData(ComponentBase::TypeID componentID);

    template<typename COMPONENT, typename STATE>
    static void Register(const char* componentName, const char* stateName)
    {
        dataSize_[COMPONENT::TypeID] = sizeof(STATE);
        typeNames_[COMPONENT::TypeID] = componentName;
        typeNameHashToIndexTable_[StringHash(componentName)] = COMPONENT::TypeID;
        //components_[COMPONENT::TypeID] = new ECSVector<ComponentBase*>(new SimpleECSVectorAlloc<COMPONENT*>());
        metaData_[COMPONENT::TypeID] = new ComponentMetaData(componentName, stateName);
    }

    static ComponentBase::TypeID GetIndexFromTypeName(const char* name);

private:
    /// Lists the visible names of all objects.
    static std::vector<std::string> typeNames_;
    /// Maps fnv1a string hashes to their corresponding entity typeID.
    static std::unordered_map<StringHash, ComponentBase::TypeID> typeNameHashToIndexTable_;
    /// Stores the sizeof(ComponentState) for all component states.
    static std::vector<size_t> dataSize_;
    /// @Deprecated: list of components. Moved to local storage in the EntityDefinition.
    static std::vector< ECSVector<ComponentBase*>* > components_;
    /// Contains the property tables for each component type.
    static std::vector< ComponentMetaData* > metaData_;
};

ComponentRegistry* Global_ComponentRegistry();