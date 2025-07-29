
#define STB_C_LEXER_IMPLEMENTATION
#include "stb_c_lexer.h"

#include "Types.h"

#include <iostream>
#include <cstdio>
#include <cstdint>

#ifdef WIN32
#include <Windows.h>
#include <tchar.h> // Bullshit
#endif

#include <assert.h>
#include <functional>
#include <map>
#include <sstream>
#include <string>
#include <vector>

/// Mark a type as reflected, include additional info inside of the type info
#define REFLECTED(...)

/// Mark a global variable to be exposed
/* example usage: 
        REFLECT_GLOBAL(Editor)
        static MapData* currentMapData;  
    By exposing that to editor will be able to see all reflected material in MapData, including the std::vector<MapObject*> contents contained

    Can also be placed on a method to expose the return value
*/
#define REFLECT_GLOBAL(...)

/// Properties are exposed by default
/*
    This can be used to define additional traits for the property:
        name "Pretty Name To Print"
        tip "Textual usage tip"
        depend "FieldName"  (GUI hint: changing this field means having to refresh all fields with "depend")
        Precise (GUI hint: should use 0.01 for steps, instead of 1.0 default)
        Fine    (GUI hint: should use 0.1 for steps, instead of 1.0 default)
        Q16     (GUI hint: fixed-point, using Q16.16, must be int)
        Q32     (GUI hint: fixed-point using Q32.32, must be int64)
        get __GetterMethodName__ (BINDING: getter must be TYPE FUNCTION() const)
        set __SetterMethodName__ (BINDING: setter must be void FUNCTION(const TYPE&) )
        resource __ResourceMember__ (BINDING: named property is the holder for resource data that matches this resource handle object)
*/
#define PROPERTY(...)
#define VIRTUAL_PROPERTY(...)

/// Next property will not be reflected
#define NO_REFLECT

/// Specify an enumeration to use for a bitfiled
#define BITFIELD_FLAGS(ENUM_NAME)

/// Bind a method for GUI exposure
/*
    name "Pretty Name To Print"
    tip "Textual usage tip"
    editor (command will be exposed in the editor GUI, otherwise it is assumed to be for scripting only)
*/
#define METHOD_COMMAND(METHOD_INFO)

struct Reflectable
{

};

REFLECTED()
struct TestStruct
{    
    // Non-networked property
    PROPERTY(name "Value A")
    int valueA_;

    // Networked property
    PROPERTY(name "Value B" networked)
    float valueB_;

    // Test ignoring reflection
    PROPERTY(networked)
    std::vector<int> integerVec_;

    // 
    PROPERTY(networked)
    int testArray_[32];

    // Bind function for code-graph calling
    METHOD_COMMAND()
    void TestFunction(int value);

    // Bind 'java-bean' like property that doesn't really exist
    VIRTUAL_PROPERTY("Virtual Value" float get GetVirtualValue set SetVirtualValue)
    int GetVirtualValue() const;
    void SetVirtualValue(int);
};

void TestStruct::TestFunction(int value)
{

}

METHOD_COMMAND()
void TestGlobal()
{

}

typedef std::function<void(stb_lexer*, long)> OnTokenCall;

REFLECT_GLOBAL()
unsigned curlyDepth_ = 0;
REFLECT_GLOBAL()
unsigned parenDepth_ = 0;
REFLECT_GLOBAL()
unsigned arrayDepth_ = 0;
REFLECT_GLOBAL()
unsigned templateDepth_ = 0;

struct LastLexerData {
    int token = 0;
    std::string text;
    int intLit = 0;
    float realLit = 0.0f;
} lexerHistory_[16];

#define LEXER_HISTORY_LENGTH 16

void PushLexerHistory(stb_lexer* lexer)
{
    for (unsigned i = 1; i < 16; ++i)
        lexerHistory_[i] = lexerHistory_[i - 1];
    
    // set th emost recent item
    lexerHistory_[0].token = lexer->token;

    lexerHistory_[0].text.clear();
    if (lexer->token == CLEX_id || CLEX_intlit)
        lexerHistory_[0].text = lexer->string;

    lexerHistory_[0].intLit = 0;
    if (lexer->token == CLEX_intlit)
        lexerHistory_[0].intLit = lexer->int_number;

    lexerHistory_[0].realLit = 0.0f;
    if (lexer->token == CLEX_floatlit)
        lexerHistory_[0].realLit = lexer->real_number;
}

/// Use these methods to avoid accidentally mangling the depth counters, should never need to set outside of lexer-calls
unsigned GetCurlyDepth() { return curlyDepth_; }
unsigned GetParenDepth() { return parenDepth_; }
unsigned GetArrayDepth() { return arrayDepth_; }
unsigned GetTemplateDepth() { return templateDepth_; }

std::vector<OnTokenCall> lexerCalls_ = {
    [](stb_lexer* lexer, long code) { // Depth handler
        if (code == '{')
            ++curlyDepth_;
        else if (code == '}')
            --curlyDepth_;
        else if (code == '(')
            ++parenDepth_;
        else if (code == ')')
            --parenDepth_;
        else if (code == '[')
            ++arrayDepth_;
        else if (code == ']')
            --arrayDepth_;
    },
    [](stb_lexer* lexer, long code) {
        PushLexerHistory(lexer);
    }
};

/// Use this method for all lexer advances, takes care of recording the last N tokens/values and making lexer callbacks for depth tracking
int AdvanceLexer(stb_lexer* lexer)
{
    if (lexer->token == CLEX_eof)
        return 0;
    if (lexer->parse_point >= lexer->eof)
        return 0;
    int code = stb_c_lexer_get_token(lexer);
    if (code)
    {
        for (auto lexCall : lexerCalls_)
            lexCall(lexer, code);
        return lexer->token;
    }
    else
        return 0;
}

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
    while (AdvanceLexer(lexer) != tokenCode)
        ss << CLEX_ToString(lexer);
    return ss.str();
}

static bool ReadNameOrModifiers(stb_lexer* lexer, unsigned& modifiers, std::string& name)
{
    bool ret = false;

    while (AdvanceLexer(lexer) == CLEX_id)
    {
        if (strcmp(lexer->string, "public") == 0)
        {
            modifiers &= AM_Public;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "private") == 0)
        {
            modifiers &= AM_Private;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "internal") == 0)
        {
            modifiers &= AM_Internal;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "abstract") == 0)
        {
            modifiers &= AM_Abstract;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "const") == 0)
        {
            modifiers &= AM_Const;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "mutable") == 0)
        {
            modifiers &= AM_Mutable;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "volatile") == 0)
        {
            modifiers &= AM_Volatile;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "thread_local") == 0)
        {
            modifiers &= AM_ThreadLocal;
            ret = true;
            continue;
        }
        else if (strcmp(lexer->string, "extern") == 0)
        {
            modifiers &= AM_Extern;
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

/// Retrieve some type information, this is aware of basic type modifiers and namespaces
static ReflectedType* GetTypeInformation(stb_lexer* lexer, ReflectionDatabase* database, std::vector<ReflectedType*>* templateParams = 0x0)
{
    ReflectedType* foundType = 0x0;
    bool isConst = false;
    bool isPointer = false;
    bool insideTemplate = false;
    
    std::string name;

    if (lexer->token == CLEX_id)
    {
        if (strcmp(lexer->string, "const") == 0)
        {
            isConst = true;
            AdvanceLexer(lexer);
        }
        name = lexer->string;
        if (auto found = database->GetType(name.c_str()))
            foundType = found;
    }

    AdvanceLexer(lexer);

    // deal with namespaces (std::vector, std::map)
    if (lexer->token == ':')
    {
        AdvanceLexer(lexer);
        if (lexer->token == ':')
        {
            name += "::";
            AdvanceLexer(lexer);
            name += lexer->string;
            AdvanceLexer(lexer);

            if (!foundType)
            {
                if (auto found = database->GetType(name.c_str()))
                    foundType = found;
            }
        }
    }

    if (lexer->token == '*')
    {
        isPointer = true;
        AdvanceLexer(lexer);
    }

    // template
    if (lexer->token == '<')
    {
        AdvanceLexer(lexer);
        std::vector<ReflectedType*> templateTypes;
        do {
            ReflectedType* templateType = GetTypeInformation(lexer, database);
            if (templateType)
                templateTypes.push_back(templateType);
        } while (lexer->token != ',' && lexer->token != '>');

        if (templateParams)
            *templateParams = templateTypes;
        AdvanceLexer(lexer);
    }

    return foundType;
}

static Property* GetPropertyTypeInformation(stb_lexer* lexer, ReflectionDatabase* database)
{
    return 0x0;
}

static void ReadMember(stb_lexer* lexer, ReflectedType* forType, ReflectionDatabase* database, std::vector<ReflectedType*>& typeStack, const std::vector<std::string>& injectBinding = std::vector<std::string>())
{
    ReflectedType* bitNames = 0x0;

    // Hit a 'do not reflect' indicator?
    if (strcmp(lexer->string, "NO_REFLECT") == 0)
    {
        // not processing so skip until we hit a semi-colon delimiter
        while (AdvanceLexer(lexer) && lexer->token != ';'); // still covers the NO_REFLECT() 
        return;
    }

// Processing binding traits if present
    std::vector<std::string> bindingTraits = injectBinding;

    bool allowMethodProcessing = false;
    // Hit property or function info annotation?
    if (strcmp(lexer->string, "PROPERTY") == 0 || (allowMethodProcessing = (strcmp(lexer->string, "METHOD_COMMAND") == 0)) || strcmp(lexer->string, "REFLECT_GLOBAL") == 0)
    {
        allowMethodProcessing = true;
        AdvanceLexer(lexer); // eat the ( from METHOD_COMMAND(

        // TODO read interior data
        while (AdvanceLexer(lexer) && lexer->token == CLEX_id || lexer->token == CLEX_dqstring) // is a sqstring even possible in C? Escapes?
            bindingTraits.push_back(lexer->string);

        if (lexer->token == ')')
            AdvanceLexer(lexer); // eat the ) from METHOD_COMMAND()
    }
    else if (strcmp(lexer->string, "VIRTUAL_PROPERTY") == 0)
    {
        AdvanceLexer(lexer); // eat the ( from VIRTUAL_PROPERTY(

        while (AdvanceLexer(lexer) && lexer->token == CLEX_id || lexer->token == CLEX_dqstring)
            bindingTraits.push_back(lexer->string);

        if (lexer->token == ')')
            AdvanceLexer(lexer);

        std::vector<ReflectedType*> templateParams;
        if (auto foundType = database->GetType(bindingTraits[1]))
        {
            assert(bindingTraits.size() >= 4);
            Property* property = new Property();
            property->isVirtual_ = true;
            property->propertyName_ = bindingTraits[0];
            property->type_ = foundType;
            property->bindingData_ = bindingTraits;
            property->templateParameters_ = templateParams;
            if (forType)
                forType->properties_.push_back(property);
            else
                database->globalProperties_.push_back(property);
        }
    }

    if (strcmp(lexer->string, "BITFIELD_FLAGS") == 0)
    {
        AdvanceLexer(lexer); // eat the "("
        if (AdvanceLexer(lexer) && lexer->token == CLEX_id) // get the BITFIELD_FLAGS(myEnum)
        {
            if (auto found = database->GetType(lexer->string))
                bitNames = found;
        }
        AdvanceLexer(lexer); // eat the ")"
    }


// We're now at the "int bob;" part
    /*
    
    cases to handle:
        int data;               // easy case
        int data = 3;           // default initialization ... initializer is grabbed as a string and placed literally in the generated code
        int* data;              // pointers ...   only for function
        int& data;              // references ... only for functions
        const int jim;          // const-ness ... only for functions
        shared_ptr<Roger> bob;  // templates

    special function cases to handle:
        void SimpleFunc();                      // No return and no arguments
        int SimpleFunc();                       // Has a return value
        int SimpleFunc() const;                 // Is a constant method
        void ArgFunc(int argumnet);             // Has an argument
        int ArgFunc(int argumnet = 1);          // Has an argument with default value
        int ArgFunc(const int* argumnet = 0x0); // Has a complex argument
    */

    std::vector<ReflectedType*> templateParams;
    if (auto foundType = GetTypeInformation(lexer, database, &templateParams))
    {
        std::string name;
        //if (AdvanceLexer(lexer)) // already there because of GetTypeInformation call
        {
            if (lexer->token == CLEX_id)
                name = lexer->string;
        }

        if (!AdvanceLexer(lexer))
            return;

        if (lexer->token == '(') // function
        {
            // They aren't automatically bound
            // requiring binding allows making some strictness that wouldn't fly in 
            // an autobinding situation, not handling everything under the sun
            if (allowMethodProcessing)
            {
                Method* newMethod = new Method();
                newMethod->methodName_ = name;
                newMethod->returnType_ = foundType;
                newMethod->bindingData_ = bindingTraits;
                if (forType)
                    forType->methods_.push_back(newMethod);
                else
                    database->globalFunctions_.push_back(newMethod);

                // Process the function argument types
                while (AdvanceLexer(lexer) && lexer->token != ')' && lexer->token != ';')
                {
                    bool isConst = false;
                    if (strcmp(lexer->string, "const") == 0)
                    {
                        isConst = true;
                        AdvanceLexer(lexer);
                    }

                    // get the argument
                    Property* prop = new Property();
                    newMethod->argumentTypes_.push_back(prop);
                    if (auto functionArgType = GetTypeInformation(lexer, database))
                        prop->type_ = functionArgType;

                    bool isPointer = false;
                    bool isReference = false;
                    if (lexer->token == '*')
                    {
                        prop->accessModifiers_ = prop->accessModifiers_ & AM_Pointer;
                        AdvanceLexer(lexer);
                    }
                    else if (lexer->token == '&')
                    {
                        prop->accessModifiers_ = prop->accessModifiers_ & AM_Reference;
                        AdvanceLexer(lexer);
                    }
                    
                    if (lexer->token == CLEX_id)
                    {
                        newMethod->argumentNames_.push_back(lexer->string);
                        AdvanceLexer(lexer);
                    }

                    while (lexer->token != ',' && lexer->token != ')')
                    {
                        if (lexer->token == '=')
                        {
                            AdvanceLexer(lexer);
                            newMethod->defaultArguments_.push_back(lexer->string);
                        }
                        else
                            AdvanceLexer(lexer);
                    }

                    // make sure we're always the right size
                    while (newMethod->defaultArguments_.size() < newMethod->argumentTypes_.size())
                        newMethod->defaultArguments_.push_back(std::string());
                    while (newMethod->argumentNames_.size() < newMethod->argumentTypes_.size())
                        newMethod->argumentNames_.push_back(std::string());
                }
            }
            else
            {
                // not processing method, eat until we hit a semicolon
                while (AdvanceLexer(lexer) && lexer->token != ';');
            }
        }
        else if (lexer->token == '[') // array
        {
            Property* property = new Property();
            property->propertyName_ = name;
            property->enumSource_ = bitNames;
            property->type_ = foundType;
            property->bindingData_ = bindingTraits;
            if (forType)
                forType->properties_.push_back(property);
            else
                database->globalProperties_.push_back(property);

            if (AdvanceLexer(lexer))
            {
                if (lexer->token == CLEX_intlit)
                    property->arraySize_ = lexer->int_number;
                else if (lexer->token == CLEX_id)
                    property->arraySize_ = database->GetPossibleLiteral(lexer->string);
            }

            AdvanceLexer(lexer);
            assert(lexer->token == ']');
        }
        else
        {
            Property* property = new Property();
            property->propertyName_ = name;
            property->type_ = foundType;
            property->bindingData_ = bindingTraits;
            property->templateParameters_ = templateParams;
            if (forType)
                forType->properties_.push_back(property);
            else
                database->globalProperties_.push_back(property);

            // Property has default value
            if (lexer->token == '=')
            {
                //TODO: get default value
            }
            else
            {
                // had better be on the semi-colon
                assert(lexer->token == ';');
            }
        }
    }
    else
        return;
}

static ReflectedType* ProcessStruct(stb_lexer* lexer, bool isClass, ReflectionDatabase* database, bool defaultIsPublic, std::vector<ReflectedType*>& typeStack)
{
    bool inPublicScope = defaultIsPublic;

    ReflectedType* type = new ReflectedType();
    std::string typeName;
    if (AdvanceLexer(lexer) && lexer->token == CLEX_id)
        type->typeName = lexer->string;

    // Eat until we get to the opening of the class/struct, or an indicator of inheritance
    while (AdvanceLexer(lexer) && (lexer->token != ':' && lexer->token != '{'))
        continue;

    // Deal with baseclass
    if (lexer->token == ':')
    {
        unsigned accessModifiers = 0;
        std::string baseName;
        if (ReadNameOrModifiers(lexer, accessModifiers, baseName))
        {
            if (!baseName.empty())
            {
                if (auto found = database->GetType(baseName.c_str()))
                    type->baseClass_.push_back(found);
                else
                {
                    // push a type to resolve later
                    ReflectedType* baseType = new ReflectedType();
                    type->baseClass_.push_back(baseType);
                    baseType->typeName = baseName;
                    baseType->isComplete = false;
                }
            }
        }
    }

    // double check of '}' is deliberate, call ordering
    while (lexer->token != '}' && AdvanceLexer(lexer) && lexer->token != '}')
    {
        if (lexer->token == CLEX_id && strcmp(lexer->string, "public") == 0)
        {
            AdvanceLexer(lexer); // eat the : from public:
            inPublicScope = true;
        }
        else if (lexer->token == CLEX_id && (strcmp(lexer->string, "protected") == 0 || strcmp(lexer->string, "private") == 0))
        {
            AdvanceLexer(lexer); // eat the : from private:
            inPublicScope = false;
        }

        // only read things we can possibly access, we have no way of presently of knowing about friendships
        if (inPublicScope)
            ReadMember(lexer, type, database, typeStack);

        while (lexer->token != ';' && lexer->token != CLEX_eof)
            AdvanceLexer(lexer);
    }

    return type;
}

static ReflectedType* ProcessEnum(stb_lexer* lexer, ReflectionDatabase* db)
{
    std::string typeName;

    bool isEnumClass = false;
    if (AdvanceLexer(lexer) && lexer->token == CLEX_id)
    {
        if (strcmp(lexer->string, "class") == 0)
        {
            isEnumClass = true;
            AdvanceLexer(lexer);
        }
        typeName = lexer->string;
    }

    // couldn't get a type name, junk it
    if (typeName.empty())
        return 0x0;

    ReflectedType* ret = new ReflectedType();
    ret->typeName = typeName;

    if (isEnumClass)
    {
        AdvanceLexer(lexer);
        if (lexer->token == ':')
        {
            AdvanceLexer(lexer);
            ret->enumType_ = GetTypeInformation(lexer, db);
        }
    }

    // Eat tokens until we hit the opening
    if (lexer->token != '{')
    {
        while (AdvanceLexer(lexer) && lexer->token != '{')
            continue;
    }

    // enums start at 0
    int lastEnumValue = 0;
    std::string valueName = "";

    while (AdvanceLexer(lexer))
    {
        if (lexer->token == '}') // hit end of enum
            break;

        int value = INT_MIN;
        if (lexer->token == CLEX_id && valueName.empty()) // name of enum
        {
            valueName = lexer->string;
            continue; // repeat to check for =
        }
        else if (lexer->token == '=') // assignment, value of enum
        {
            bool hasValue = false;
            while (AdvanceLexer(lexer))
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
                    if (AdvanceLexer(lexer) && lexer->token == CLEX_intlit)
                    {
                        value <<= lexer->int_number;
                        break;
                    }
                }
            }
        }

        if (value != INT_MIN && !valueName.empty())
        {
            lastEnumValue = value;
            ret->enumValues_.push_back(std::make_pair(valueName, value));
        }
        else if (!valueName.empty())
        {
            value = lastEnumValue;
            ++lastEnumValue;
            ret->enumValues_.push_back(std::make_pair(valueName, value));
        }
        valueName.clear();
    }

    if (ret->enumValues_.empty())
    {
        delete ret;
        return 0x0;
    }
    return ret;
}

static void ProcessReflected(stb_lexer* lexer, ReflectionDatabase* database, std::vector<ReflectedType*>& typeStack)
{
    std::vector<std::string> bindingInfo;

    // Grab meta data first
    while (AdvanceLexer(lexer))
    {
        if (lexer->token == CLEX_id || lexer->token == CLEX_dqstring)
            bindingInfo.push_back(lexer->string);
        else if (lexer->token == ')') // end of metablock
            break;
        else if (lexer->token == '(')
            continue;
    }

// Get our basic type information
    while (AdvanceLexer(lexer))
    {
        if (lexer->token == CLEX_id)
        {
            if (strcmp(lexer->string, "struct") == 0)
            {
                if (auto type = ProcessStruct(lexer, false, database, true, typeStack))
                    database->types_[type->typeName] = type;
            }
            else if (strcmp(lexer->string, "enum") == 0)
            {
                if (auto type = ProcessEnum(lexer, database))
                    database->types_[type->typeName] = type;
            }
            else if (strcmp(lexer->string, "class") == 0)
            {
                if (auto type = ProcessStruct(lexer, true, database, false, typeStack))
                    database->types_[type->typeName] = type;
            }
            return;
        }
    }
}

static void RunLexer(stb_lexer* lexer, ReflectionDatabase* database, std::vector<ReflectedType*>& typeStack)
{
    // keep parsing tokens
    while (AdvanceLexer(lexer))
    {
        if (lexer->token == CLEX_id)
        {
            if (strcmp(lexer->string, "REFLECTED") == 0)
            {
                // Look for a bound enum or struct
                ProcessReflected(lexer, database, typeStack);
            }
            else if (strcmp(lexer->string, "METHOD_COMMAND") == 0)
            {
                // Process a global method
                ReadMember(lexer, 0x0, database, typeStack);
            }
            else if (strcmp(lexer->string, "REFLECT_GLOBAL") == 0)
            {
                // Process a global variable
                ReadMember(lexer, 0x0, database, typeStack);
            }
        }
    }
}

std::string PrintCode(ReflectedType* type);
std::string PrintCalls(ReflectedType* type);
std::string PrintImgui(ReflectedType* type, ReflectionDatabase* database);

#ifdef WIN32
int _tmain(int argc, _TCHAR* argv[])
{
    FILE* file = fopen("C:/dev/ParsECS/TypeAnnotate/Test.cpp", "r");
    if (file)
    {
        fseek(file, 0, SEEK_END);
        uint64_t fileSize = ftell(file);
        fseek(file, 0, SEEK_SET);
        if (fileSize > 0)
        {
            char* buffer = new char[fileSize + 1];
            memset(buffer, 0, fileSize + 1);
            if (int sizeRead = fread(buffer, sizeof(char), fileSize, file))
            {
                char lexerStorage[1024];
                memset(lexerStorage, 0, sizeof lexerStorage);

                ReflectionDatabase database;

                // fill primitive types
                {
#define PRIM(NAME, VARTYPE, VARDEF) { ReflectedType* t = new ReflectedType(#NAME); database.types_[#NAME] = t; t->isPrimitive = true; t->variantType_ = #VARTYPE; t->variantDefault_ = #VARDEF; }
#define COMPLEX(NAME, VARTYPE, VARDEF) { ReflectedType* t = new ReflectedType(#NAME); database.types_[#NAME] = t; t->variantType_ = #VARTYPE; t->variantDefault_ = #VARDEF; }
#define TEMPLATE(NAME, VARTYPE, VARDEF) { ReflectedType* t = new ReflectedType(#NAME); database.types_[#NAME] = t; t->isTemplate = true; }

                    PRIM(void, VOID, 0x0);
                    PRIM(uint8_t, VT_Byte, 0);
                    PRIM(int16_t, VT_Int16, 0);
                    PRIM(int, VT_Int, 0);
                    PRIM(int64_t, VT_Int64, 0);
                    PRIM(uint16_t, VT_UInt16, 0);
                    PRIM(unsigned, VT_UInt, 0);
                    PRIM(uint64_t, VT_UInt64, 0);
                    PRIM(float, VT_Float, 0.0f);
                    PRIM(double, VT_Double, 0.0);
                    PRIM(bool, VT_Bool, false);
                    PRIM(EntityID, VT_UInt, 0);
                    PRIM(CompID, VT_UInt, 0);
                    PRIM(ResourceHandle, VT_ResourceHandle, ResourceHandle());
                    PRIM(std::string, VT_String, std::string());
                    PRIM(StringList, VT_StringList, StringList());
                    TEMPLATE(std::vector, VT_VariantVector, VariantVector());
                    TEMPLATE(std::map, VT_VariantMap, VariantMap());
                    TEMPLATE(std::set, VT_VariantVector, VariantVector());
                    TEMPLATE(std::unordered_map, VT_VariantMap, VariantMap());
                    TEMPLATE(std::unordered_set, VT_VariantVector, VariantVector());
                }

                stb_lexer lexer;
                stb_c_lexer_init(&lexer, buffer, buffer + fileSize, lexerStorage, 1024);

                std::vector<ReflectedType*> typeStack;
                RunLexer(&lexer, &database, typeStack);
                for (auto record : database.types_)
                {
                    if (record.second->isPrimitive)
                        continue;
                    else if (!record.second->enumValues_.empty()) // don't write enums here either
                        continue;

                    std::cout << PrintImgui(record.second, &database);
                    std::cout << std::endl << std::endl;
                    std::cout << PrintCode(record.second);
                    std::cout << std::endl;
                    std::cout << PrintCalls(record.second);
                    std::cout << std::endl;
                }
            }
            delete[] buffer;
        }
        fclose(file);
    }

    return 0;
}
#endif

