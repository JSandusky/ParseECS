#include "EntityDatabase.h"

#include "Entity.h"
#include "EntityDefinition.h"

EntityDefinition* EntityDatabase::GetEntityDefinition(DefID id)
{
    auto found = entityTable_.find(id);
    if (found != entityTable_.end())
        return found->second;
    return 0x0;
}

void EntityDatabase::GetEntityDefinitions(const ComponentBits& mask, std::vector<EntityDefinition*>& holder) const
{
    const size_t count = mask.count();
    for (auto record : entityTypes_)
        if ((mask & record->mask_).count() == count)
            holder.push_back(record);
}

void EntityDatabase::GetEntityDefinitions(const Aspect& aspect, std::vector<EntityDefinition*>& holder) const
{
    for (auto record : entityTypes_)
        if (aspect.Passes(record->mask_))
            holder.push_back(record);
}

ComponentBase* EntityDatabase::GetComponent(CompID typeID, DefID entityID)
{
    if (typeID < components_.size())
        return (*components_[typeID])[entityID];
    return 0x0;
}

ComponentBase* EntityDatabase::GetComponent(CompID typeID, EntityDefinition* entity)
{
    if (typeID < components_.size())
        return (*components_[typeID])[entity->id_];
    return 0x0;
}