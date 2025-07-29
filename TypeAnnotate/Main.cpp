
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

#include "Types.h"
#include "Parsing.h"
#include "Generators.h"

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
#define BITFIELD_FLAGS(...)

/// Bind a method for GUI exposure
/*
    name "Pretty Name To Print"
    tip "Textual usage tip"
    editor (command will be exposed in the editor GUI, otherwise it is assumed to be for scripting only)
*/
#define METHOD_COMMAND(...)

static Property* GetPropertyTypeInformation(stb_lexer* lexer, ReflectionDatabase* database)
{
    return 0x0;
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

    unsigned incompletes = database->CountIncompleteTypes();
    database->ResolveIncompleteTypes();
    unsigned unresolved = database->CountIncompleteTypes();
    unsigned resolved = incompletes - unresolved;
    std::cout << std::endl << "Incompletes: " << incompletes << ", Unresolved: " << unresolved << ", Resolved: " << resolved << std::endl << std::endl;
}

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

