#include "EntityManager.h"

#include "../Components/Component.h"
#include "../Components/ComponentRegistry.h"
#include "EntityDatabase.h"
#include "EntityDefinition.h"
#include "../MemoryAllocator.h"
#include "../SimWorld.h"

EntityManager::Callback::Callback(EntityManager* manager, size_t listOffset, std::function<void(Entity*)> function) : 
    manager_(manager),
    function_(function),
    listOffset_(listOffset)
{
}

EntityManager::Callback::~Callback()
{
    EntityManager::CallbackList* list = (EntityManager::CallbackList*)(manager_ + listOffset_);
    if (list)
    {
        auto& l = *list;
        auto found = std::find(l.begin(), l.end(), this);
        if (found != l.end())
            l.erase(found);
    }
}

Entity* EntityManager::CreateEntity(DefID id)
{
    assert(id);

    if (auto entityDef = EntityDatabase::GetInstance()->GetEntityDefinition(id))
    {
        return CreateEntity(entityDef);
    }
    assert(0);
    return 0x0;
}

Entity* EntityManager::CreateEntity(EntityDefinition* definition)
{
    assert(definition);

    Entity* ret = AllocateEntity();
    ret->defId_ = definition->id_;
    ret->components_ = 0x0;
    ret->mask_ = definition->mask_;
    if (inExecution_)
        pendingAddition_.push_back(ret);
    else
        FillNewEntity(ret, definition);

    return ret;
}

void EntityManager::DestroyEntity(Entity* entity)
{
    assert(entity);
    if (entity)
        DestroyEntity(entity->id_);
}

void EntityManager::DestroyEntity(EntityID entity)
{
    auto actualIndex = indirectionTable_.find(entity);
    if (actualIndex == indirectionTable_.end())
        return;

    // usual case
    if (entities_.size() > 1)
    {
        auto& tail = entities_[listTail_];
        indirectionTable_[tail.second] = actualIndex->second;
        std::swap(entities_[actualIndex->second], tail);
        
        --listTail_;
        actualIndex->second = -1;
    }
    else
    {
        --listTail_;
    }
}

Entity* EntityManager::GetEntity(uint32_t id)
{
    auto actualIndex = indirectionTable_.find(id);
    if (actualIndex != indirectionTable_.end())
        return entities_[actualIndex->second].first;
    return 0x0;
}

void EntityManager::GetEntities(const ComponentBits& mask, std::vector<Entity*>& entities)
{
    const size_t count = mask.count();
    for (unsigned i = 0; i < listTail_; ++i)
    {
        auto& ent = entities_[i];
        if ((ent.first->mask_ & mask).count() == count)
            entities.push_back(ent.first);
    }
}

void EntityManager::GetEntities(const Aspect& aspect, std::vector<Entity*>& entities)
{
    for (unsigned i = 0; i < listTail_; ++i)
    {
        auto& ent = entities_[i];
        if (aspect.Passes(ent.first->mask_))
            entities.push_back(ent.first);
    }
}

void EntityManager::for_each(const ComponentBits& mask, std::function<void(Entity*)> function)
{
    for (unsigned i = 0; i < listTail_; ++i)
    {
        auto& ent = entities_[i];
        if ((ent.first->mask_ & mask).count())
            function(ent.first);
    }
}

void EntityManager::for_each(const Aspect& aspect, std::function<void(Entity*)> function)
{
    for (unsigned i = 0; i < listTail_; ++i)
    {
        auto& ent = entities_[i];
        if (aspect.Passes(ent.first->mask_))
            function(ent.first);
    }
}

void EntityManager::for_each(DefID defID, std::function<void(Entity*)> function)
{
    for (unsigned i = 0; i < listTail_; ++i)
    {
        auto& ent = entities_[i];
        if (ent.first->defId_ == defID)
            function(ent.first);
    }
}

void EntityManager::for_each(EntityDefinition* definition, std::function<void(Entity*)> function) 
{ 
    for_each(definition->id_, function); 
}

void EntityManager::ResolvePending()
{
    // Process removals first, because addition events may result in growing arrays
    for (auto& entity : pendingRemoval_)
    {

    }

    for (auto& entity : pendingAddition_)
    {

    }

    pendingRemoval_.clear();
    pendingAddition_.clear();
}

Entity* EntityManager::AllocateEntity()
{
    if (listTail_ < entities_.size())
    {
        auto record = entities_[listTail_];
        indirectionTable_[record.first->id_] = listTail_ - 1;
        ++listTail_;
        return record.first;
    }    
    
    return 0x0;
}

void EntityManager::FillNewEntity(Entity* entity, EntityDefinition* definition)
{
    //TODO allocate entity data
    entity->components_ = (ComponentState*)world_->GetMemoryManager()->Allocate(definition->stateSize_);
    
    assert(entity->components_);
    // did allocation fail?

    size_t offset = 0;
    for (auto comp : definition->components_)
    {
        void* addr = ((unsigned char*)entity->components_) + offset;
        comp->_InitializeState(addr);
        offset += comp->StateSize();
    }
}

void EntityManager::PromoteEntity(Entity* entity, EntityDefinition* fromDefinition, EntityDefinition* toDefinition)
{
    assert(entity && fromDefinition && toDefinition);

    // do we need to do anything?
    if (entity->components_ && fromDefinition != toDefinition)
    {
        // Is this entity really changing?
        if (fromDefinition->mask_ != toDefinition->mask_)
        {
            void* readData = entity->components_;
            void* newData = 0x0;
            if (toDefinition->stateSize_ > fromDefinition->stateSize_)
                newData = world_->GetMemoryManager()->Allocate(toDefinition->stateSize_);
            else
                newData = new char[toDefinition->stateSize_];

            ComponentBits compUnion = fromDefinition->mask_ & toDefinition->mask_;
            if (compUnion.any())
            {
                size_t fromOffset = 0, toOffset = 0;
                for (unsigned i = 0, offset = 0; i < compUnion.size(); ++i)
                {
                    if (compUnion[i])
                    {
                        toDefinition->GetComponent(i)->_ConvertState(fromDefinition->GetComponent(i), (char*)readData + fromOffset, (char*)newData + toOffset);
                        fromOffset += ComponentRegistry::GetDataSize(i);
                        toOffset += ComponentRegistry::GetDataSize(i);
                    }
                    else if (fromDefinition->mask_[i])
                        fromOffset += ComponentRegistry::GetDataSize(i);
                    else if (toDefinition->mask_[i])
                    {
                        // Prepare new component data
                        toDefinition->GetComponent(i)->_InitializeState((char*)readData + offset);
                        toOffset += ComponentRegistry::GetDataSize(i);
                    }
                }
            }
            if (toDefinition->stateSize_ > fromDefinition->stateSize_)
            {
                memcpy(entity->components_, newData, toDefinition->stateSize_);
                delete[] newData;
            }
            else
            {
                world_->GetMemoryManager()->Free(entity->components_);
                entity->components_ = (ComponentState*)newData;
            }
        }
        else
        {
            // we're promoting to component identical entity
            size_t offset = 0;
            for (size_t i = 0; i < toDefinition->mask_.size(); ++i)
            {
                if (toDefinition->mask_[i])
                {
                    toDefinition->GetComponent(i)->_ConvertState(fromDefinition->GetComponent(i), (char*)entity->components_ + offset, (char*)entity->components_ + offset);
                    offset += ComponentRegistry::GetDataSize(i);
                }
            }
        }
    }
    else if (entity)
        FillNewEntity(entity, toDefinition);
}

EntityManager::Callback* EntityManager::SubscribeEntityAdded(std::function<void(Entity*)> function)
{
    auto call = new EntityManager::Callback(this, offsetof(EntityManager, entityAdded_), function);
    entityAdded_.push_back(call);
    return call;
}

EntityManager::Callback* EntityManager::SubscribeEntityRemoved(std::function<void(Entity*)> function)
{
    auto call = new EntityManager::Callback(this, offsetof(EntityManager, entityRemoved_), function);
    entityRemoved_.push_back(call);
    return call;
}

EntityManager::Callback* EntityManager::SubscribeEntityPromoted(std::function<void(Entity*)> function)
{
    auto call = new EntityManager::Callback(this, offsetof(EntityManager, entityPromoted_), function);
    entityPromoted_.push_back(call);
    return call;
}