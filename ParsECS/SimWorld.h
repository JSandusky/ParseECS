#pragma once

#include <vector>

class ComponentManager;
class ComponentRegistry;
class EntityDatabase;
class EntityManager;
class EntitySystem;
struct MemoryMan;

class SimWorld
{
public:

    EntityDatabase* GetEntityDatabase() { return 0x0; }
    ComponentRegistry* GetComponentRegistry() { return 0x0; }
    MemoryMan* GetMemoryManager() { return 0x0; }

private:
    std::vector<EntitySystem*> systems_;
    EntityManager* entityManager_;
    ComponentManager* componentManager_;
    MemoryMan* memoryManager_;
};