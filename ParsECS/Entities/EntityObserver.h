#pragma once

#include "EntityManager.h"

struct Entity;

class EntityObserver
{
public:
    EntityObserver() { }
    virtual ~EntityObserver();

    virtual void Connect(EntityManager* manager);

    virtual void EntityAdded(Entity*) = 0;
    virtual void EntityRemoved(Entity*) = 0;
    virtual void EntityPromoted(Entity*) = 0;

protected:
    std::vector<EntityManager::Callback*> callbacks_;
};