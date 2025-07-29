#pragma once

#include "../ParsecDef.h"

#include "../Aspect.h"
#include "../ECSVector.h"
#include "../Singleton.h"

#include <cstdint>
#include <unordered_map>

struct EntityDefinition;
struct ComponentBase;

/// Stores the shared data for the entity types that can be found in the world
class EntityDatabase : public Singleton<EntityDatabase>
{
public:

    /// Get an entity definition by id.
    EntityDefinition* GetEntityDefinition(DefID id);

    /// Query for entities that exactly match the given mask.
    void GetEntityDefinitions(const ComponentBits& mask, std::vector<EntityDefinition*>& holder) const;
    /// Query for entities for which an aspect passes.
    void GetEntityDefinitions(const Aspect& aspect, std::vector<EntityDefinition*>& holder) const;

    ComponentBase* GetComponent(CompID typeID, DefID entityID);
    ComponentBase* GetComponent(CompID typeID, EntityDefinition* entity);

    template<typename T>
    T* GetComponent(uint32_t entityID) {
        return (T*)GetComponent(T::TypeID, entityID);
    }

    template<typename T>
    T* GetComponent(EntityDefinition* entity) {
        return (T*)GetComponent(T::TypeID, entity);
    }

private:
    /// Maps the EntityDefinition.tag_ to the row in the entity table
    std::unordered_map<uint32_t, EntityDefinition*> entityTable_;
    std::vector<EntityDefinition*> entityTypes_;
    std::vector< ECSVector<ComponentBase*>* > components_;
};