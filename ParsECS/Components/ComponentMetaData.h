#pragma once

#include "../Aspect.h"
#include "../Reflection/ECSProperty.h"
#include "../StrHash.h"
#include <string>

#define BEGIN_METADATA(TYPENAME) { auto metaData = Global_ComponentRegistry()->GetMetaData(TYPENAME::TypeID)
#define BEGIN_COMPONENT_PROPERTIES() { auto& properties = metaData->componentProperties_
#define BEGIN_STATE_PROPERTIES() { auto& properties = metaData->stateProperties_

#define REGISTER_PROPERTY_MEMORY(TYPENAME, PROPTYPE, OFFSET, DEFVAL, NAME, DESCR, FLAGS) properties.push_back(new ECSProperty(NAME, DESCR, new MemoryPropertyAccessor<PROPTYPE, PROPTYPE>(OFFSET, DEFVAL), FLAGS))
#define REGISTER_ENUM(TYPENAME, PROPTYPE, OFFSET, DEFVAL, NAME, DESC, FLAGS, NAMES) properties.push_back(new ECSProperty(NAME, DESCR, new MemoryPropertyAccessor<PROPTYPE, PROPTYPE>(OFFSET), FLAGS, NAMES))
#define REGISTER_ACCESSOR(TYPENAME, PROPTYPE, GETTER, SETTER, DEFVAL, NAME, DESC, FLAGS)
#define REGISTER_CONST_ACCESSOR(TYPENAME, PROPTYPE, GETTER, SETTER, DEFVAL, NAME, DESC, FLAGS)

#define END_PROPERTIES() }
#define END_METADATA() }

struct ComponentMetaData
{
    ComponentMetaData(const char* compName, const char* stateName) :
        name_(compName),
        typeNameHash_(compName)
    {
    }

    /// Hash of the C++ side type name at registration, should be the class name for coherency.
    StringHash typeNameHash_;
    /// String version of the C++ side type name.
    std::string name_;
    /// Optional name for pretty printing informat.
    std::string prettyName_;
    /// Optional description of the purpose of the component.
    std::string description_;

    /// Presence of this component requires the listed components.
    ComponentBits requiresBits_;
    /// The component cannot coexist with these components.
    ComponentBits forbiddenBits_;
    /// The component is allowed as long as at least one of these components is present.
    ComponentBits anyBits_;

    /// Registered list of reflected properties for the Shared Component.
    std::vector<ECSProperty*> componentProperties_;
    /// Registered list of reflected properties for the component state.
    std::vector<ECSProperty*> stateProperties_;

    void RegisterComponentProperty(ECSProperty* property);
    void RegisterStateProperty(ECSProperty* property);
};