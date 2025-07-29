#include "ConcernedList.h"

#include "ComponentCount.h"

#include <algorithm>

ConcernedList::ConcernedList(const Aspect& aspect) :
    aspect_(aspect)
{
}

void ConcernedList::SortList()
{
    if (size() > sortingMin_ && size() < sortingMax_)
    {
        if (sortDefinition_)
            std::sort(begin(), end(), [](const ConcernedEntity& lhs, const ConcernedEntity& rhs) {
            if (lhs.defID_ < rhs.defID_)
                return true;
            return lhs.dataAddr_ < rhs.dataAddr_;
        });
        else
            std::sort(begin(), end(), [](const ConcernedEntity& lhs, const ConcernedEntity& rhs) {
            return lhs.dataAddr_ < rhs.dataAddr_;
        });
    }
}

void ConcernedList::AddSystem(EntitySystem* system)
{
    using_.push_back(system);
}

void ConcernedList::RemoveSystem(EntitySystem* system)
{
    auto found = std::find(using_.begin(), using_.end(), system);
    if (found != using_.end())
        using_.erase(found);
}

void ConcernedList::EntityAdded(Entity* entity)
{
    if (entity && aspect_.Passes(entity->mask_))
        push_back({ entity->id_, entity->defId_, entity->components_ });
}

void ConcernedList::EntityRemoved(Entity* entity)
{
    if (entity && aspect_.Passes(entity->mask_))
    {
        //auto found = std::find(using_.begin(), using_.end(), ConcernedEntity { entity->id_, entity->defId_, entity->components_ });
        //if (found != using_.end())
        //{
        //    std::swap(*found, using_.back());
        //    using_.pop_back();
        //}
    }
}

void ConcernedList::EntityPromoted(Entity* entity)
{

}