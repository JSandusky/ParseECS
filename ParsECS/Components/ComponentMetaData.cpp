#include "ComponentMetaData.h"


void ComponentMetaData::RegisterComponentProperty(ECSProperty* property)
{
    componentProperties_.push_back(property);
}

void ComponentMetaData::RegisterStateProperty(ECSProperty* property)
{
    stateProperties_.push_back(property);
}