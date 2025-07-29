
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#include <cstdio>
#include <cstdint>

#ifdef WIN32
#include <Windows.h>
#include <tchar.h> // Bullshit
#endif

#include <map>
#include <sstream>
#include <string>
#include <vector>

#define REFLECTED

REFLECTED
struct TestStruct
{
    int valueA_;
    float valueB_;
};

REFLECTED
enum AccessModifiers
{
    AM_Default = 0,
    AM_Public = 1,
    AM_Protected = 1 << 1,
    AM_Private = 1 << 2,
    AM_Internal = 1 << 3,
    AM_Abstract = 1 << 4,
    AM_Virtual = 1 << 5,
    AM_Override = 1 << 6,
    AM_Const = 1 << 7
};

struct ReflectedType;

/// Stringify the current token of the lexer
std::string CLEX_ToString(stb_lexer* lexer)
{
    if (lexer->token == CLEX_andand) return "&&";
    else if (lexer->token == CLEX_andeq) return "&=";
    else if (lexer->token == CLEX_arrow) return "->";
    else if (lexer->token == CLEX_charlit) return "" + ((char)lexer->token);
    else if (lexer->token == CLEX_diveq) return "/=";
    else if (lexer->token == CLEX_dqstring) return "\"" + std::string(lexer->string) + "\"";
    else if (lexer->token == CLEX_eq) return "==";
    else if (lexer->token == CLEX_floatlit) return std::to_string(lexer->real_number); // TODO
    else if (lexer->token == CLEX_greatereq) return ">=";
    else if (lexer->token == CLEX_id) return lexer->string;
    else if (lexer->token == CLEX_intlit) return std::to_string(lexer->int_number); // TODO
    else if (lexer->token == CLEX_lesseq) return "<=";
    else if (lexer->token == CLEX_minuseq) return "-=";
    else if (lexer->token == CLEX_minusminus) return "--";
    else if (lexer->token == CLEX_modeq) return "%=";
    else if (lexer->token == CLEX_muleq) return "*=";
    else if (lexer->token == CLEX_noteq) return "!=";
    else if (lexer->token == CLEX_oreq) return "|=";
    else if (lexer->token == CLEX_oror) return "||";
    else if (lexer->token == CLEX_pluseq) return "+=";
    else if (lexer->token == CLEX_plusplus) return "++";
    else if (lexer->token == CLEX_shl) return "<<";
    else if (lexer->token == CLEX_shleq) return "<<=";
    else if (lexer->token == CLEX_shr) return ">>";
    else if (lexer->token == CLEX_shreq) return ">>=";
    else if (lexer->token == CLEX_xoreq) return "^=";
    return "" + ((char)lexer->token);
}

/// Grab as string everything up until we hit an expected token code
/// Used for grabbing initializers for members and the values of enums (to avoid needing to run a full parser)
/// For enums:
///     CLEX_ToStringUntil(lexer, ',') ... but we have to initialize these things somehow
/// For member defaults:
///     CLEX_ToStringUntil(lexer, ';')
std::string CLEX_ToStringUntil(stb_lexer* lexer, long tokenCode)
{
    std::stringstream ss;
    while (stb_c_lexer_get_token(lexer) != tokenCode)
        ss << CLEX_ToString(lexer);
    return ss.str();
}

struct Property
{
    unsigned accessModifiers_ = 0;
    std::string propertyName_ = "";
    ReflectedType* type_ = 0x0;
};

struct ReflectedType
{
    /// Includes template factors
    std::string typeName;
    /// Mapped as <access_modifiers, Type>
    std::vector<std::pair<unsigned, ReflectedType* > > baseClass_;
    ReflectedType* stateType_ = 0x0;
    /// If not null then we're a template type
    ReflectedType* templateParameterType_ = 0x0;
    /// List of the members of the type
    std::vector<Property*> properties_;
    /// If not empty then the type is an enum
    std::vector< std::pair<std::string, int> > enumValues_;
};

struct ReflectionDatabase
{
    std::map<std::string, ReflectedType*> types_;
};

static bool ReadNameOrModifiers(stb_lexer* lexer, unsigned& modifiers, std::string& name)
{
    bool ret = false;

    while (stb_c_lexer_get_token(lexer) == CLEX_id)
    {
        if (strcmp(lexer->string, "public"))
        {
            modifiers &= AM_Public;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "private"))
        {
            modifiers &= AM_Private;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "internal"))
        {
            modifiers &= AM_Private;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "abstract"))
        {
            modifiers &= AM_Private;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "const"))
        {
            modifiers &= AM_Private;
            ret = true;
            continue;
        }
        else
        {
            name = lexer->string;
            break;
        }
        break;
    }
    return ret;
}

static void ReadProperty(stb_lexer* lexer, ReflectedType* forType, ReflectionDatabase* database)
{
    Property* property = new Property();
    // anything that starts with void is a function for sure
    if (strcmp(lexer->string, "void"))
        return;

    auto foundType = database->types_.find(lexer->string);
    if (foundType != database->types_.end())
    {
        std::string memberName;
        // we've identified the type of the property
        while (stb_c_lexer_get_token(lexer))
        {
            if (lexer->token == CLEX_id) // member-name
            {
                memberName = lexer->string;
            }
            else if (lexer->token == '<') // start of a template
            {
                if (stb_c_lexer_get_token(lexer) == CLEX_id) // template type
                {
                    if (stb_c_lexer_get_token(lexer) != '>') // eat the tail of the token
                        return;
                }
            }
            else if (lexer->token == '=' && !memberName.empty()) // default value
            {
                
            }
            else
                return;
        }
    }
    else
        return;
}

static ReflectedType* ProcessStruct(stb_lexer* lexer, bool isClass, ReflectionDatabase* database)
{
    ReflectedType* type = new ReflectedType();
    std::string typeName;
    if (stb_c_lexer_get_token(lexer) && lexer->token == CLEX_id)
        typeName = lexer->string;

    // Eat until we get to a baseclass
    while (stb_c_lexer_get_token(lexer) && lexer->token != ':')
        continue;

    unsigned accessModifiers = 0;
    std::string baseName;
    if (ReadNameOrModifiers(lexer, accessModifiers, baseName))
    {
        // Eat tokens until we hit the opening
        while (stb_c_lexer_get_token(lexer) && lexer->token != '{')
            continue;
        ReadProperty(lexer, type, database);
    }

    return type;
}

static ReflectedType* ProcessEnum(stb_lexer* lexer)
{
    std::string typeName;

    if (stb_c_lexer_get_token(lexer) && lexer->token == CLEX_id)
        typeName = lexer->string;

    // couldn't get a type name, junk it
    if (typeName.empty())
        return 0x0;

    ReflectedType* ret = new ReflectedType();
    ret->typeName = typeName;

    // Eat tokens until we hit the opening
    while (stb_c_lexer_get_token(lexer) && lexer->token != '{') 
        continue;

    // enums start at 0
    int lastEnumValue = 0;
    std::string valueName = "";

    while (stb_c_lexer_get_token(lexer))
    {
        if (lexer->token == '}') // hit end of enum
            break;

        int value = INT_MIN;
        if (lexer->token == CLEX_id && valueName.empty()) // name of enum
            valueName = lexer->string;
        else if (lexer->token == '=') // assignment, value of enum
        {
            bool hasValue = false;
            while (stb_c_lexer_get_token(lexer))
            {
                if (lexer->token == '(')
                    continue;
                else if (lexer->token == ')') // terminate if at end of quote
                    break;
                else if (lexer->token == ',') // terminate if hit comma
                    break;
                else if (lexer->token == '}') // terminate if hit enum close
                    break;

                if (lexer->token == CLEX_intlit && value == INT_MIN)
                    value = lexer->int_number;
            
                if (lexer->token == CLEX_shl) // bitwise shift?
                {
                    if (stb_c_lexer_get_token(lexer) && lexer->token == CLEX_intlit)
                    {
                        value << lexer->int_number;
                        break;
                    }
                }
            }
        }

        if (value != INT_MIN)
        {
            lastEnumValue = value;
            ret->enumValues_.push_back(std::make_pair(valueName, value));
        }
        else
        {
            value = lastEnumValue;
            ++lastEnumValue;
            ret->enumValues_.push_back(std::make_pair(valueName, value));
        }
    }

    if (ret->enumValues_.empty())
    {
        delete ret;
        return 0x0;
    }
    return ret;
}

static void ProcessReflected(stb_lexer* lexer, ReflectionDatabase* database)
{
    // Grab meta data first
    while (stb_c_lexer_get_token(lexer))
    {
        if (lexer->token == CLEX_id)
        {
            if (strcmp(lexer->string, "") == 0)
            {

            }
        }
        else if (lexer->token == ')') // end of metablock
            break;
    }

    while (stb_c_lexer_get_token(lexer))
    {
        if (lexer->token == CLEX_id)
        {
            if (strcmp(lexer->string, "struct"))
                if (auto type = ProcessStruct(lexer, false, database))
                    database->types_[type->typeName] = type;
            else if (strcmp(lexer->string, "enum"))
                if (auto type = ProcessEnum(lexer))
                    database->types_[type->typeName] = type;
            else if (strcmp(lexer->string, "class"))
                if (auto type = ProcessStruct(lexer, true, database))
                    database->types_[type->typeName] = type;
        }
    }
}

static void RunLexer(stb_lexer* lexer, ReflectionDatabase* database)
{
    // keep parsing tokens
    while (stb_c_lexer_get_token(lexer))
    {
        if (lexer->token == CLEX_id)
        {
            // are we a bound enum?
            if (strcmp(lexer->string, "REFLECTED") == 0)
                ProcessReflected(lexer, database);
        }
    }
}


#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
{
    FILE* file = fopen("Main.cpp", "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        uint64_t fileSize = ftell(file);
        if (fileSize > 0)
        {
            char* buffer = new char[fileSize + 1];
            buffer[fileSize] = 0;
            if (fread(buffer, sizeof(char), fileSize, file) == fileSize)
            {
                char lexerStorage[1024];
                memset(lexerStorage, 0, sizeof lexerStorage);

                ReflectionDatabase database;
                stb_lexer lexer;
                stb_c_lexer_init(&lexer, buffer, buffer + fileSize, lexerStorage, 1024);
                RunLexer(&lexer, &database);
            }
            delete[] buffer;
        }
        fclose(file);
    }

    return 0;
}
#endif

std::string PrintCode(ReflectedType* type)
{
    std::stringstream ss;

    ss << "void Register" << type->typeName << "(ReflectionRegistry* reg) {\r\n";

    ss << "    REGISTER_COMPONENT(" << type->typeName << ", " << type->stateType_->typeName << ")\r\n";
    ss << "    BEGIN_METADATA(" << type->typeName << ")\r\n";
    ss << "        BEGIN_COMPONENT_PROPERTIES()\r\n";

    ReflectedType* t = type;
    for (auto property : type->properties_)
    {
        ss << "            REGISTER_PROPERTY_MEMORY(";
        ss << t->typeName << ", " << property->type_->typeName << ", " << "offsetof(" << t->typeName << ", " << property->propertyName_ << "), ";
        ss << "DEFAULT" << ", \"" << property->propertyName_ << "\"";
        ss << ")\r\n";
    }

    ss << "        END_PROPERTIES()\r\n";
    ss << "        BEGIN_STATE_PROPERTIES()\r\n";
    
    t = type->stateType_;
    for (auto property : type->stateType_->properties_)
    {
        ReflectedType* type = type->stateType_;
        ss << "            REGISTER_PROPERTY_MEMORY(";
        ss << t->typeName << ", " << property->type_->typeName << ", " << "offsetof(" << t->typeName << ", " << property->propertyName_ << "), ";
        ss << "DEFAULT" << ", \"" << property->propertyName_ << "\"";
        ss << ")\r\n";
    }

    ss << "        END_PROPERTIES()\r\n";
    ss << "    END_METADATA()\r\n";

    ss << "}\r\n";

    return ss.str();
}