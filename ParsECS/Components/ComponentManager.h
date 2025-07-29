#pragma once

#include "Component.h"
#include "../ECSVector.h"

#include <cstdint>

class ConcernedList;

/// Manages the instantiated components for a SimWorld
class ComponentManager
{
public:
    inline ComponentBase* GetComponent(uint32_t typeID, uint32_t index) { return (*components_[typeID])[index]; }

private:
    std::vector< ECSVector<ComponentBase*>* > components_;
    std::vector< ConcernedList*> concernedLists_;
};