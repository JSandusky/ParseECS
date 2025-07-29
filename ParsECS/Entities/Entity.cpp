#include "Entity.h"

#include "../Components/Component.h"
#include "../Components/ComponentRegistry.h"
#include "EntityDefinition.h"

ComponentState* Entity::GetComponentState(CompID index)
{
    size_t offset = 0;
    for (unsigned i = 0; i < mask_.size(); ++i)
        if (mask_[i] && i != index)
            offset += ComponentRegistry::GetDataSize(i);
        else if (i == index)
            return (ComponentState*)((char*)components_ + offset);
    return 0x0;
}

ComponentState* Entity::GetComponentState(const char* typeName)
{
    uint32_t typeIndex = ComponentRegistry::GetIndexFromTypeName(typeName);
    if (typeIndex == -1)
        return 0x0;
    return GetComponentState(typeIndex);
}

ComponentBase* Entity::GetComponent(CompID bitIndex)
{
    EntityDefinition* def = 0x0;
    if (def)
        return def->GetComponent(bitIndex);
    return 0x0;
}

ComponentBase* Entity::GetComponent(const char* typeName)
{
    EntityDefinition* def = 0x0;
    if (def)
        return def->GetComponent(typeName);
    return 0x0;
}