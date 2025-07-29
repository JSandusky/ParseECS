#pragma once

#include "../Aspect.h"
#include "../ConcernedList.h"
#include "../Entities/Entity.h"

#include <vector>

/// Baseclass for systems that operate on entities
class EntitySystem
{
public:
    EntitySystem();
    virtual ~EntitySystem();

    void SetConcerning(ConcernedList* list) { concerns_ = list; list->AddSystem(this); }
    ConcernedList* GetConcerns() { return concerns_; }
    const ConcernedList* GetConcerns() const { return concerns_; }

protected:
    /// The list of concerned entities is potentially shared
    ConcernedList* concerns_;
};