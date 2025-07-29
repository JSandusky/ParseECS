#include "EntityObserver.h"

#include "EntityManager.h"

#include <functional>

EntityObserver::~EntityObserver()
{
    for (unsigned i = 0; i < callbacks_.size(); ++i)
        delete callbacks_[i];
}

void EntityObserver::Connect(EntityManager* manager)
{
    if (!manager)
        return;
    callbacks_.push_back(manager->SubscribeEntityAdded([=](Entity* e) { this->EntityAdded(e); }));
    callbacks_.push_back(manager->SubscribeEntityRemoved([=](Entity* e) { this->EntityRemoved(e); }));
    callbacks_.push_back(manager->SubscribeEntityPromoted([=](Entity* e) { this->EntityPromoted(e); }));
}