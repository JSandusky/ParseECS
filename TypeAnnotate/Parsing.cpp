#include "Parsing.h"

#include <assert.h>
#include <sstream>

#ifdef _DEBUG
    #define VERIFY(a)  assert(a)
#else
    #define VERIFY(a) a
#endif

std::string Isolate(const std::string& code, const std::string& openKey, const std::string& closeKey)
{
    std::stringstream ss;

    auto start = code.find_first_of(openKey);
    auto end = code.find_first_of(closeKey);

    if (start != std::string::npos && end != std::string::npos)
    {
        do {
            ss << code.substr(start + openKey.length(), start - end + openKey.length());

            start = code.find_first_of(openKey, start + 1);
            end = code.find_first_of(closeKey, end + 1);
        } while (start != std::string::npos && end != std::string::npos);
    }
    else
        return code; // no delimiters, return what we have

    return ss.str(); // return what we've accumulated
}

/// Stringify the current token of the lexer
std::string CLEX_ToString(stb_lexer* lexer)
{
    if (lexer->token == CLEX_andand) return "&&";
    else if (lexer->token == CLEX_andeq) return "&=";
    else if (lexer->token == CLEX_arrow) return "->";
    else if (lexer->token == CLEX_charlit) return "" + ((char)lexer->token);
    else if (lexer->token == CLEX_diveq) return "/=";
    else if (lexer->token == CLEX_dqstring) return "\"" + std::string(lexer->string, lexer->string_len) + "\"";
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
    return std::string() + ((char)lexer->token);
}

/// Grab as string everything up until we hit an expected token code
/// Used for grabbing initializers for members and the values of enums (to avoid needing to run a full parser)
/// For enums:
///     CLEX_ToStringUntil(lexer, ',') ... but we have to initialize these things somehow
/// For member defaults:
///     CLEX_ToStringUntil(lexer, ';')
std::string CLEX_ToStringUntil(stb_lexer* lexer, std::vector<long> tokenCodes, bool scope)
{
    int scopeDepth = 0;
    std::stringstream ss;
    do {
        AdvanceLexer(lexer);
        if (scopeDepth > 0 && scope)
        {
        }
        else if (std::find(tokenCodes.begin(), tokenCodes.end(), lexer->token) != tokenCodes.end())
            break;

        if ((lexer->token == '(' || lexer->token == '{') && scope)
            ++scopeDepth;
        if ((lexer->token == ')' || lexer->token == '}') && scope)
            --scopeDepth;
        ss << CLEX_ToString(lexer);

    } while (true);


    //while (std::find(tokenCodes.begin(), tokenCodes.end(), AdvanceLexer(lexer)) == tokenCodes.end() || scopeDepth > 0)
    //{
    //    if ((lexer->token == '(' || lexer->token == '{') && scope)
    //        ++scopeDepth;
    //    if ((lexer->token == ')' || lexer->token == '}') && scope)
    //        --scopeDepth;
    //    ss << CLEX_ToString(lexer);
    //}
    return ss.str();
}

#define LEXER_HISTORY_LENGTH 16

struct LastLexerData {
    int token = 0;
    std::string text;
    int intLit = 0;
    float realLit = 0.0f;
} lexerHistory_[16];

void PushLexerHistory(stb_lexer* lexer)
{
    for (unsigned i = 1; i < 15; ++i)
        lexerHistory_[i] = lexerHistory_[i - 1];

    // set th emost recent item
    lexerHistory_[0].token = lexer->token;

    lexerHistory_[0].text.clear();
    if (lexer->token == CLEX_id || lexer->token == CLEX_intlit || lexer->token == CLEX_floatlit || lexer->token == CLEX_dqstring || lexer->token == CLEX_charlit)
        lexerHistory_[0].text = lexer->string;

    lexerHistory_[0].intLit = 0;
    if (lexer->token == CLEX_intlit)
        lexerHistory_[0].intLit = lexer->int_number;

    lexerHistory_[0].realLit = 0.0f;
    if (lexer->token == CLEX_floatlit)
        lexerHistory_[0].realLit = lexer->real_number;
}

typedef std::function<void(stb_lexer*, long)> OnTokenCall;

std::vector<OnTokenCall> lexerCalls_ = {
    [](stb_lexer* lexer, long code) {
        PushLexerHistory(lexer);
    }
};

int PeekToken(stb_lexer* lexer)
{
    stb_lexer lex = *lexer;
    if (stb_c_lexer_get_token(&lex))
        return lex.token;
    return 0;
}

std::string PeekText(stb_lexer* lexer)
{
    stb_lexer lex = *lexer;
    if (stb_c_lexer_get_token(&lex))
    {
        if (lex.token == CLEX_id)
            return std::string(lex.string);
    }
    return "";
}

void EatBlock(stb_lexer* lexer, char openToken, char closeToken)
{
    int depth = 0;
    while (stb_c_lexer_get_token(lexer))
    {
        if (lexer->token == openToken)
        {
            ++depth;
        }
        else if (lexer->token == closeToken)
        {
            --depth;
            if (depth == 0)
                return;
        }
    }
}

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
            lexCall(lexer, lexer->token);
        return lexer->token;
    }
    else
        return 0;
}

// Should be of ( .... ) parenthesis bounded form
void ReadTraitsList(stb_lexer* lexer, std::vector<std::string>& dest)
{
    while (AdvanceLexer(lexer) && lexer->token == CLEX_id || lexer->token == CLEX_dqstring) // is a sqstring even possible in C? Escapes?
        dest.push_back(lexer->string);
}

bool ReadNameOrModifiers(stb_lexer* lexer, unsigned& modifiers, std::string& name)
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

ReflectedType* GetTypeInformation(stb_lexer* lexer, ReflectionDatabase* database, std::vector<ReflectedType*>* templateParams)
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

void ReadMember(stb_lexer* lexer, ReflectedType* forType, ReflectionDatabase* database, std::vector<ReflectedType*>& typeStack, const std::vector<std::string>& injectBinding)
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
        ReadTraitsList(lexer, bindingTraits);

        if (lexer->token == ')')
            AdvanceLexer(lexer); // eat the ) from METHOD_COMMAND()
    }
    else if (strcmp(lexer->string, "VIRTUAL_PROPERTY") == 0)
    {
        AdvanceLexer(lexer); // eat the ( from VIRTUAL_PROPERTY(

        ReadTraitsList(lexer, bindingTraits);

        if (lexer->token == ')')
            AdvanceLexer(lexer);

        if (auto foundType = database->GetType(bindingTraits[1]))
        {
            assert(bindingTraits.size() >= 4);
            Property* property = new Property();
            property->isVirtual_ = true;
            property->propertyName_ = bindingTraits[0];
            property->typeHandle_.type_ = foundType;
            property->bindingData_ = bindingTraits;
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

    TypeInstance foundType = ReadTypeInstance(lexer, database);

    std::string name;
    assert(lexer->token == CLEX_id);
    name = lexer->string;

    stb_lexer savedState = *lexer;
    VERIFY(AdvanceLexer(lexer));    

    if (lexer->token == '(') // function
    {
        // They aren't automatically bound
        // requiring binding allows making some strictness that wouldn't fly in 
        // an autobinding situation, not handling everything under the sun
        if (allowMethodProcessing)
        {
            Method* newMethod = new Method();
            newMethod->methodName_ = name;
            newMethod->returnTypeHandle_ = foundType;
            if (foundType.Is(AM_Static))
                newMethod->isStatic_;

            newMethod->bindingData_ = bindingTraits;
            if (forType)
                forType->methods_.push_back(newMethod);
            else
                database->globalFunctions_.push_back(newMethod);

            // Process the function argument types
            while (AdvanceLexer(lexer) && lexer->token != ')' && lexer->token != ';')
            {
                TypeInstance paramType = ReadTypeInstance(lexer, database);

                // get the argument
                Property* prop = new Property();
                newMethod->argumentTypes_.push_back(prop);
                prop->typeHandle_ = paramType;

                if (lexer->token == CLEX_id)
                {
                    prop->propertyName_ = lexer->string;
                    newMethod->argumentNames_.push_back(lexer->string);
                    AdvanceLexer(lexer);
                }

                while (lexer->token != ',' && lexer->token != ')')
                {
                    if (lexer->token == '=')
                    {
                        std::string defText = CLEX_ToStringUntil(lexer, { ',', ')', ';' }, true);
                        newMethod->defaultArguments_.push_back(defText);
                    }
                    else
                        AdvanceLexer(lexer);
                }

                // make sure we're always the right size
                while (newMethod->defaultArguments_.size() < newMethod->argumentTypes_.size())
                    newMethod->defaultArguments_.push_back(std::string());
                while (newMethod->argumentNames_.size() < newMethod->argumentTypes_.size())
                    newMethod->argumentNames_.push_back(std::string());

                if (lexer->token == ')')
                    break;
            }

            if (lexer->token == ')') // end if function
            {
                if (PeekText(lexer) == "const")
                {
                    newMethod->isConst_ = true;
                    AdvanceLexer(lexer);
                }
                if (PeekToken(lexer) == '{')
                {
                    EatBlock(lexer, '{', '}');
                    lexer->token = ';'; // forcibly set the token to ';' so the outer code deals with us
                }
            }
        }
        else
        {
            // not processing method, eat until we hit a semicolon
            *lexer = savedState;
            EatBlock(lexer, '(', ')');
            if (PeekText(lexer) == "const")
                AdvanceLexer(lexer);
            if (PeekToken(lexer) == '{')
            {
                EatBlock(lexer, '{', '}');
                lexer->token = ';'; // forcibly inject a semicolon for the code calling us
                return;
            }
            while (lexer->token != ';')
                AdvanceLexer(lexer);

                
        }
    }
    else if (lexer->token == '[') // array
    {
        Property* property = new Property();
        property->propertyName_ = name;
        property->enumSource_ = bitNames;
        property->typeHandle_ = foundType;
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
        property->typeHandle_ = foundType;
        property->bindingData_ = bindingTraits;
        if (forType)
            forType->properties_.push_back(property);
        else
            database->globalProperties_.push_back(property);

        // Property has default value
        if (lexer->token == '=')
        {    
            property->defaultValue_ = CLEX_ToStringUntil(lexer, { ',', ')', ';' }, true);
        }
        else
        {
            // had better be on the semi-colon
            assert(lexer->token == ';');
        }
    }
}

ReflectedType* ProcessStruct(stb_lexer* lexer, bool isClass, ReflectionDatabase* database, bool defaultIsPublic, std::vector<ReflectedType*>& typeStack)
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
        do {
            AdvanceLexer(lexer);

            TypeInstance baseType = ReadTypeInstance(lexer, database);
            type->baseClass_.push_back(baseType);

        } while (lexer->token == ',');
    }

    // double check of '}' is deliberate, call ordering
    while (lexer->token != '}' && AdvanceLexer(lexer) != '}')
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

        // we ended a function
        if (lexer->token == '}')
        {
            AdvanceLexer(lexer);
            continue;
        }

        // we're already on a semi-colon for some reason
        if (lexer->token == ';')
            continue;

        // this shouldn't happen should it because of the default initializer stuff?
        while (lexer->token != ';' && lexer->token != CLEX_eof)
            AdvanceLexer(lexer);
    }

    return type;
}

ReflectedType* ProcessEnum(stb_lexer* lexer, ReflectionDatabase* db)
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
        while (AdvanceLexer(lexer) != '{')
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

void ProcessReflected(stb_lexer* lexer, ReflectionDatabase* database, std::vector<ReflectedType*>& typeStack)
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

TypeInstance ReadTypeInstance(stb_lexer* lexer, ReflectionDatabase* database)
{
    TypeInstance ret;
    ReflectedType* foundType = 0x0;
    unsigned modifiers = 0;
    std::string name;

    do
    {
        if (strcmp(lexer->string, "public") == 0)
        {
            ret.accessModifiers_ |= AM_Public;
            continue;
        }
        else if (strcmp(lexer->string, "private") == 0)
        {
            ret.accessModifiers_ |= AM_Private;
            continue;
        }
        else if (strcmp(lexer->string, "internal") == 0)
        {
            ret.accessModifiers_ |= AM_Internal;
            continue;
        }
        else if (strcmp(lexer->string, "abstract") == 0)
        {
            ret.accessModifiers_ |= AM_Abstract;
            continue;
        }
        else if (strcmp(lexer->string, "const") == 0)
        {
            ret.accessModifiers_ |= AM_Const;
            continue;
        }
        else if (strcmp(lexer->string, "mutable") == 0)
        {
            ret.accessModifiers_ |= AM_Mutable;
            continue;
        }
        else if (strcmp(lexer->string, "volatile") == 0)
        {
            ret.accessModifiers_ |= AM_Volatile;
            continue;
        }
        else if (strcmp(lexer->string, "static") == 0)
        {
            ret.accessModifiers_ |= AM_Static;
            continue;
        }
        else if (strcmp(lexer->string, "thread_local") == 0)
        {
            ret.accessModifiers_ |= AM_ThreadLocal;
            continue;
        }
        else if (strcmp(lexer->string, "extern") == 0)
        {
            ret.accessModifiers_ |= AM_Extern;
            continue;
        }
        else
        {
            name = lexer->string;
            AdvanceLexer(lexer);
            break;
        }
        break;
    } while (AdvanceLexer(lexer) == CLEX_id);

    if (!name.empty())
    {
        if (auto found = database->GetType(name.c_str()))
            ret.type_ = foundType = found;
    }

    if (lexer->token == ':')
    {
        assert(!foundType);

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
                    ret.type_ = found;
            }
        }
    }

    if (lexer->token == '<')
    {
        AdvanceLexer(lexer);
        std::vector<TypeInstance*> templateTypes;
        do {
            if (lexer->token == CLEX_intlit)
            {
                ret.templateParameters_.push_back({ TypeInstance(), lexer->int_number });
                AdvanceLexer(lexer);
            }
            else
            {
                auto templateType = ReadTypeInstance(lexer, database);
                ret.templateParameters_.push_back({ templateType, INT_MAX });
            }
        } while (lexer->token != ',' && lexer->token != '>');

        AdvanceLexer(lexer);
    }

    // handle ref/pointer
    while (lexer->token == '&' || lexer->token == '*')
    {
        if (lexer->token == '&')
            ret.accessModifiers_ |= AM_Reference;
        if (lexer->token == '*')
            ret.accessModifiers_ |= AM_Pointer;
        AdvanceLexer(lexer);
    }

    if (ret.type_ == nullptr)
    {
        ret.type_ = new ReflectedType();
        ret.type_->typeName = name;
        ret.type_->isComplete = false;
    }
    return ret;
}

TypeInstance TypeInstanceFromString(const std::string& txt, ReflectionDatabase* db)
{
    static char lexerStorage[4096];
    memset(lexerStorage, 0, sizeof lexerStorage);

    stb_lexer lex;
    stb_c_lexer_init(&lex, txt.c_str(), txt.c_str() + txt.length(), lexerStorage, 4096);
    AdvanceLexer(&lex);
    return ReadTypeInstance(&lex, db);
}
