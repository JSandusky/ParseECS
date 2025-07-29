#pragma once

#include "../ParsecDef.h"

#include "../Aspect.h"
#include "Entity.h"

#include <functional>
#include <unordered_map>
#include <vector>

BEGIN_PARSECS_NS

struct EntityDefinition;
class SimWorld;

/// Manages the entities of a SimWorld. Responsible for the lifecycle and access.
class EntityManager
{
public:

    struct Callback {
        Callback(EntityManager* manager, size_t listOffset, std::function<void(Entity*)> function);
        ~Callback();
        std::function<void(Entity*)> function_;

    private:
        EntityManager* manager_ = 0x0;
        size_t listOffset_;
    };

    /// Create an entity from a definition ID.
    Entity* CreateEntity(DefID definitionId);
    /// Create an entity from a definition instance.
    Entity* CreateEntity(EntityDefinition* definition);

    /// Destroy an entity by instance.
    void DestroyEntity(Entity* entity);
    /// Destroy an entity by ID.
    void DestroyEntity(EntityID entity);

    /// Retrieve an entity by it's handle
    Entity* GetEntity(EntityID id);
    /// Retrieve entities that match a bitmask.
    void GetEntities(const ComponentBits& mask, std::vector<Entity*>& entities);
    /// Retrieve entities for which an aspect passes.
    void GetEntities(const Aspect& aspect, std::vector<Entity*>& entities);

    /// Execute a function on all entities whose bits match the mask
    void for_each(const ComponentBits& mask, std::function<void(Entity*)> function);
    /// Execute a function on on all entities for who the aspect passes.
    void for_each(const Aspect& mask, std::function<void(Entity*)> function);
    /// Execute a function object on all entities referencing the given EntityDefinition.
    void for_each(DefID defID, std::function<void(Entity*)> function);
    /// Execute a function object on all entities referencing the given EntityDefinition.
    void for_each(EntityDefinition* definition, std::function<void(Entity*)> function);

    /// Locks additions, removals, and promotions in the entity component system from being valid.
    inline void Begin() { inExecution_ = true; }
    /// Unlocks the automatic execution of additions, removals, and promotions.
    inline void End() { inExecution_ = false; }

    /// Lock-object's use stack scope to restrict blocks of actions by placing the EntityManager into inExecution state.
    struct LockObject {
        EntityManager* manager_;
        LockObject(EntityManager* manager) : manager_(manager)  { manager_->Begin(); }
        ~LockObject() { manager_->End(); }
    };

    /// Construct a lock object for this manager.
    inline LockObject LockScope() { return LockObject(this); }

    /// Processes pending additions, removals, and promotions after execution has finished.
    void ResolvePending();

    typedef std::function<void(Entity*)> CallbackFunction;
    Callback* SubscribeEntityAdded(std::function<void(Entity*)> function);
    Callback* SubscribeEntityRemoved(std::function<void(Entity*)> function);
    Callback* SubscribeEntityPromoted(std::function<void(Entity*)> function);

    SimWorld* GetWorld() const { return world_; }

private:
    /// Allocates an entity.
    Entity* AllocateEntity();
    void FillNewEntity(Entity* entity, EntityDefinition* definition);
    void PromoteEntity(Entity* entity, EntityDefinition* fromDefinition, EntityDefinition* toDefinition);

    /// The simulation world
    SimWorld* world_ = 0x0;
    /// Maps an entity handle to an actual index.
    std::unordered_map<uint32_t, uint32_t> indirectionTable_;
    /// Stores the entity instances, along with their handle index in the indirection table so that the the actual index can be updated.
    std::vector< std::pair<Entity*, uint32_t> > entities_;

    /// When execution is blocked entity creation results in queued construction.
    std::vector<Entity*> pendingAddition_;
    /// When execution is blocked entity removal results in queued destrution.
    std::vector<Entity*> pendingRemoval_;

    typedef std::vector<Callback*> CallbackList;
    CallbackList entityAdded_;
    CallbackList entityRemoved_;
    CallbackList entityPromoted_;

    /// End of the cache vector of entities, past this is either void space or entities waiting to be brought back into service.
    size_t listTail_ = 0;
    /// If true then all creates and removes and not instant
    bool inExecution_ = false;
};

END_PARSECS_NS