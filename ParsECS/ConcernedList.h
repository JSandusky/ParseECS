#pragma once

#include "Aspect.h"
#include "Entities/Entity.h"
#include "Entities/EntityManager.h"
#include "Entities/EntityObserver.h"

#include <vector>

class EntitySystem;

/// Concerned lists track collections of entities that others are interested in observing.
class ConcernedList : public std::vector <ConcernedEntity>, public EntityObserver
{
public:
    ConcernedList(const Aspect& aspect);

    void DisableSorting() { sortingMin_ = UINT_MAX; sortingMax_ = 0; }
    void SetSortingRange(size_t lowerBound, size_t upperBound) { sortingMin_ = lowerBound; sortingMax_ = upperBound; }
    bool IsSortingDefinitions() const { return sortDefinition_; }
    void SetSortingDefinitions(bool state) { sortDefinition_ = state; }

    void SortList();

    virtual void AddSystem(EntitySystem* system);
    virtual void RemoveSystem(EntitySystem* system);

    virtual void EntityAdded(Entity*) override;
    virtual void EntityRemoved(Entity*) override;
    virtual void EntityPromoted(Entity*) override;

    const std::vector<ConcernedEntity>& GetEntities() const { return *this; }

private:
    Aspect aspect_;
    /// Concerned list must be larger than this before we'll sort
    size_t sortingMin_ = 32;
    /// Concert list must be smaller than this for us to sort
    size_t sortingMax_ = UINT_MAX;
    /// Whether to sort by the entity definition first or ignore it
    bool sortDefinition_ = true;
    /// List of EntitySystems that reference this list
    std::vector<EntitySystem*> using_;
};