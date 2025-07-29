#pragma once

#include <vector>

class EntitySystem;

class SystemManager
{
public:
    SystemManager();
    virtual ~SystemManager();



private:
    std::vector<EntitySystem*> systems_;
};