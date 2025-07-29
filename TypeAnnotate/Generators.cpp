#include "Types.h"

#include <sstream>

std::string PrintCode(ReflectedType* type)
{
    if (type == 0)
        return "";
    std::stringstream ss;

    auto GetPropertyBindingName = [](Property* property) {
        if (HasBindingProperty(property->bindingData_, "name"))
            return GetBindingProperty(property->bindingData_, "name");
        return property->propertyName_;
    };

    ss << "void Register" << type->typeName << "(ReflectionRegistry* reg) {\r\n";

    if (type->stateType_)
        ss << "    REGISTER_COMPONENT(" << type->typeName << ", " << type->stateType_->typeName << ")\r\n";
    else
        ss << "    REGISTER_COMPONENT(" << type->typeName << ", 0x0)\r\n";
    ss << "    BEGIN_METADATA(" << type->typeName << ")\r\n";
    ss << "        BEGIN_COMPONENT_PROPERTIES()\r\n";

    ReflectedType* t = type;
    for (auto property : type->properties_)
    {
        if (property->isVirtual_)
        {
            std::string getter = GetBindingProperty(property->bindingData_, "get");
            std::string setter = GetBindingProperty(property->bindingData_, "set");

            if (setter.empty())
            {
                ss << "            REGISTER_PROPERTY_VIRTUAL(";
                ss << t->typeName << ", " << property->GetFullTypeName() << ", " << getter << ", " << property->propertyName_ << ", ";
                ss << "RPF_Default" << ", \"" << GetPropertyBindingName(property) << "\"";
                ss << ")\r\n";
            }
            else
            {
                ss << "            REGISTER_PROPERTY_VIRTUAL(";
                ss << t->typeName << ", " << property->GetFullTypeName() << ", " << getter << ", " << setter << ", ";
                ss << "RPF_Default" << ", \"" << GetPropertyBindingName(property) << "\"";
                ss << ")\r\n";
            }
        }
        else
        {
            ss << "            REGISTER_PROPERTY_MEMORY(";
            ss << t->typeName << ", " << property->GetFullTypeName() << ", " << "offsetof(" << t->typeName << ", " << property->propertyName_ << "), ";
            ss << "RPF_Default" << ", \"" << GetPropertyBindingName(property) << "\"";
            ss << ")\r\n";
        }
    }

    ss << "        END_PROPERTIES()\r\n";
    ss << "        BEGIN_STATE_PROPERTIES()\r\n";

    if (t = type->stateType_)
    {
        for (auto property : type->stateType_->properties_)
        {
            ss << "            REGISTER_PROPERTY_MEMORY(";
            ss << t->typeName << ", " << property->GetFullTypeName() << ", " << "offsetof(" << t->typeName << ", " << property->propertyName_ << "), ";
            ss << "RPF_Default" << ", \"" << GetPropertyBindingName(property) << "\"";
            ss << ")\r\n";
        }
    }

    ss << "        END_PROPERTIES()\r\n";
    ss << "    END_METADATA()\r\n";

    ss << "}\r\n";

    return ss.str();
}

std::string VariantGetter(std::string typeName)
{
    return "get<" + typeName + ">()";
}

std::string PrintCalls(ReflectedType* type)
{
    std::stringstream ret;
    for (auto method : type->methods_)
    {
        ret << "[](VariantVector args) {\r\n";

        ret << "    Variant ret;\r\n";
        if (method->returnTypeHandle_.GetTypeName() != "void")
            ret << "    ret = ";
        else
            ret << "    ";

        ret << method->methodName_ << "(";
        for (unsigned i = 0; i < method->argumentTypes_.size(); ++i)
        {
            if (i > 0)
                ret << ", ";
            auto argType = method->argumentTypes_[i];
            ret << "args.size() > " << i << " ? args[" << i << "]." << VariantGetter(argType->typeHandle_.GetTypeName()) << " : " << argType->typeHandle_.type_->variantDefault_;
        }
        ret << ");\r\n";
        ret << "    return ret;\r\n";
        ret << "}\r\n";
    }

    return ret.str();
}



std::string PrintImgui(ReflectedType* type, ReflectionDatabase* database)
{
    if (type == 0)
        return "";
    std::stringstream ss;

    auto GetPropertyBindingName = [](Property* property) {
        if (HasBindingProperty(property->bindingData_, "name"))
            return GetBindingProperty(property->bindingData_, "name");
        return property->propertyName_;
    };

    ss << "void OnGui(" << type->typeName << "* object) {\r\n";

    for (auto property : type->properties_)
    {
        auto textName = GetPropertyBindingName(property);
        auto propName = property->propertyName_;
        if (property->isVirtual_)
        {

        }
        else
        {
            if (property->typeHandle_.Is(database->GetType("int")))
            {
                if (property->arraySize_ > 0)
                {
                    ss << "    if (ImGui::CollapsingHeader(\"" << textName << "\") {\r\n";
                    ss << "        ImGui::Indent();\r\n";
                    for (int i = 0; i < property->arraySize_; ++i)
                        ss << "        ImGui::DragInt(\"" << textName << "[" << i << "]\", &object->" << propName << "[" << i << "]);\r\n";
                    ss << "        ImGui::Unindent();\r\n";
                    ss << "    }\r\n";
                }
                else
                    ss << "    ImGui::DragInt(\"" << textName << "\", &object->" << propName << ");\r\n";
            }
            else if (property->typeHandle_.Is(database->GetType("float")))
            {
                ss << "    ImGui::DragFloat(\"" << textName << "\", &object->" << propName << ");\r\n";
            }
            else if (property->typeHandle_.Is(database->GetType("std::string")))
            {
                ss << "{\r\n"
                    "    static char data[4096];\r\n"
                    "    memcpy(data, object->" << propName << ".c_str(), object->" << propName << ".length());\r\n" 
                    "    if (ImGui::InputText(\"" << textName << "\", data, 4096))\r\n"
                    "        object->" << propName << " = data;\r\n"
                    "}\r\n";
            }
        }
    }

    for (auto method : type->methods_)
    {
        if (method->argumentTypes_.size() == 0)
        {
            ss << "    if (imgui::button(\"" << method->methodName_ << "\")\r\n";
            ss << "        object->" << method->methodName_ << "();\r\n";
        }
    }

    ss << "}\r\n";

    return ss.str();
}

std::string GenerateFunctionDefs(ReflectionDatabase* db, const std::string& fwd)
{
    std::stringstream ss;
    ss << "// AUTOGENERATED CODE - DO NOT MODIFY" << std::endl;
    ss << fwd;
    for (auto m : db->globalFunctions_)
    {
        if (m->HasBindingSwitch("gendef"))
        {
            ss << m->returnTypeHandle_.ToString(true);
            ss << " " << m->methodName_;
            ss << "(";
            for (unsigned i = 0; i < m->argumentTypes_.size(); ++i)
            {
                if (i > 0)
                    ss << ", ";
                ss << m->argumentTypes_[i]->typeHandle_.ToString(true);
                if (!m->argumentTypes_[i]->propertyName_.empty())
                    ss << " " << m->argumentTypes_[i]->propertyName_;
            }
            ss << ")";
            if (m->isConst_)
                ss << " const;";
            else
                ss << ";";
            ss << std::endl;
        }
    }
    return ss.str();
}