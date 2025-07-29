#pragma once

#include "../ParsecDef.h"
#include "../ComponentCount.h"

#include <stdint.h>
#include <bitset>
#include <vector>

struct ComponentState;
struct ComponentBase;

struct Entity
{
    EntityID id_ = -1;
    DefID defId_ = -1;
    ComponentBits mask_;
    ComponentState* components_ = 0x0;

    ComponentState* GetComponentState(CompID index);
    ComponentState* GetComponentState(const char* typeName);

    template<typename T>
    T* GetComponentState() { return (T*)GetComponent(index); }

    ComponentBase* GetComponent(CompID bitIndex);
    ComponentBase* GetComponent(const char* typeName);

    template<typename T>
    T* GetComponent() {
        return (T*)GetComponent(T::TypeID);
    }
};

/// Used for things that need to store references to entities.
/// The defID and dataAddress allow systems/observers to sort their lists based on different needs
struct ConcernedEntity
{
    EntityID entityID_;
    DefID defID_;
    ComponentState* dataAddr_;
};