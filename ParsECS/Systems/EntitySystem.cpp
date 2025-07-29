#include "EntitySystem.h"

#include "../ComponentCount.h"

#include <algorithm>


EntitySystem::EntitySystem()
{

}

EntitySystem::~EntitySystem()
{
    if (concerns_)
        concerns_->RemoveSystem(this);
}