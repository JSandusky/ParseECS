#include "EntityDefinition.h"

#include "Entity.h"
#include "../Components/Component.h"
#include "../Components/ComponentRegistry.h"

EntityDefinition::EntityDefinition()
{

}

EntityDefinition::~EntityDefinition()
{
    for (unsigned i = 0; i < components_.size(); ++i)
        delete components_[i];
    components_.clear();
}

ComponentBase* EntityDefinition::GetComponent(CompID bitIndex)
{
    return components_[FlatIndex(mask_, bitIndex)];
}

ComponentBase* EntityDefinition::GetComponent(const char* typeName)
{
    uint32_t idx = Global_ComponentRegistry()->GetIndexFromTypeName(typeName);
    if (idx != -1)
        return GetComponent(idx);
    return 0x0;
}

ComponentBase* EntityDefinition::AddComponent(CompID compID)
{
    assert(!(flags_ & EDF_Sealed), "Attempting to add components to a sealed entity");
    if (!(flags_ & EDF_Sealed))
        return 0x0;

    ComponentBase* newInstance = 0x0;
    if (newInstance == 0x0)
        return 0x0;

    // Setup our updated mask
    mask_.set(compID, true);
    size_t insertIndex = FlatIndex(mask_, compID);
    components_.insert(components_.begin() + insertIndex, newInstance);
    // TODO: calculate allocation size

    return newInstance;
}

ComponentBase* EntityDefinition::AddComponent(const char* typeName)
{
    assert(!(flags_ & EDF_Sealed), "Attempting to add components to a sealed entity");
    if (!(flags_ & EDF_Sealed))
        return 0x0;

    uint32_t foundID = Global_ComponentRegistry()->GetIndexFromTypeName(typeName);
    if (foundID != -1)
        return AddComponent(foundID);

    return 0x0;
}

void EntityDefinition::RemoveComponent(CompID compID)
{
    assert(!(flags_ & EDF_Sealed), "Attempting to remove components from a sealed entity");
    if (!(flags_ & EDF_Sealed))
        return;

    if (!mask_.test(compID))
        return;

    size_t eraseIndex = FlatIndex(mask_, compID);
    // Todo send to memory free
    components_.erase(components_.begin() + eraseIndex);

    mask_.set(compID, false);
    // TODO recalculate our stateSize_
}

void EntityDefinition::RemoveComponent(const char* typeName)
{
    assert(!(flags_ & EDF_Sealed), "Attempting to remove components from a sealed entity");
    if (!(flags_ & EDF_Sealed))
        return;

    uint32_t foundID = Global_ComponentRegistry()->GetIndexFromTypeName(typeName);
    if (foundID != -1)
        RemoveComponent(foundID);

    return;
}