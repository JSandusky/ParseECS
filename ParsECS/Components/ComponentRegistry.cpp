#include "ComponentRegistry.h"

#include "../ComponentCount.h"

std::vector<uint32_t> ComponentRegistry::dataSize_(PARSECS_COMPONENT_COUNT);

std::vector<std::string> ComponentRegistry::typeNames_(PARSECS_COMPONENT_COUNT);

std::vector< ECSVector<ComponentBase*>* > ComponentRegistry::components_(PARSECS_COMPONENT_COUNT, 0);

std::unordered_map<StringHash, ComponentBase::TypeID> ComponentRegistry::typeNameHashToIndexTable_ = std::unordered_map<StringHash, ComponentBase::TypeID>();

std::vector<ComponentMetaData*> ComponentRegistry::metaData_ = std::vector<ComponentMetaData*>(PARSECS_COMPONENT_COUNT, 0);

ComponentRegistry* Global_ComponentRegistry() {
    return 0x0;
}

ComponentBase::TypeID ComponentRegistry::GetIndexFromTypeName(const char* name)
{
    auto found = typeNameHashToIndexTable_.find(name);
    if (found != typeNameHashToIndexTable_.end())
        return found->second;
    return -1;
}

ComponentMetaData* ComponentRegistry::GetMetaData(ComponentBase::TypeID typeID)
{
    return metaData_[typeID];
}