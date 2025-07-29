#pragma once

#include "../ParsecDef.h"

#include <algorithm>
#include <cstdint>
#include <type_traits>

struct ComponentState
{
};

struct ComponentBase
{    
    typedef uint32_t TypeID;

    virtual size_t StateSize() const = 0;
    virtual CompID GetTypeID() const = 0;

protected:
    friend class EntityManager;
    virtual void _InitializeState(void* state) = 0;
    virtual void _ConvertState(ComponentBase* oldComponent, void* fromState, void* toState) = 0;
};

template<typename STATE>
struct Component : public ComponentBase
{
    typedef STATE State;

    static const CompID TypeID;

    /// Duck-typing.
    virtual size_t StateSize() const override { return sizeof(State); }
    /// Duck-typing.
    virtual CompID GetTypeID() const override { return TypeID; }
    
    /// Default behaviour is only placement new.
    virtual void InitializeState(STATE* state) { new (state) State; }
    /// Default behaviour is a total wipe.
    virtual void ConvertState(Component* from, STATE* fromState, STATE* toState) { fromState->~State(); new (toState)State; }

private:
    virtual void _InitializeState(void* state) { InitializeState((STATE*)state); }
    virtual void _ConvertState(ComponentBase* fromComp, void* fromState, void* toState) override { ConvertState((Component*)fromComp, (STATE*)fromState, (STATE*)toState); }
};

