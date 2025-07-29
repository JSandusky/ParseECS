#pragma once

#include "Types.h"

#include <functional>
#include <map>
#include <string>
#include <vector>

#define STB_C_LEXER_DEFINITIONS   
#include "stb_c_lexer.h"

/// Use this method for all lexer advances, takes care of recording the last N tokens/values and making lexer callbacks for depth tracking
int AdvanceLexer(stb_lexer* lexer);

/// Stringify the current token of the lexer
std::string CLEX_ToString(stb_lexer* lexer);

/// Grab as string everything up until we hit an expected token code
/// Used for grabbing initializers for members and the values of enums (to avoid needing to run a full parser)
/// For enums:
///     CLEX_ToStringUntil(lexer, ',') ... but we have to initialize these things somehow
/// For member defaults:
///     CLEX_ToStringUntil(lexer, ';')
std::string CLEX_ToStringUntil(stb_lexer* lexer, long tokenCode);

ReflectedType* GetTypeInformation(stb_lexer* lexer, ReflectionDatabase* database, std::vector<ReflectedType*>* templateParams = 0x0);

void ReadMember(stb_lexer* lexer, ReflectedType* forType, ReflectionDatabase* database, std::vector<ReflectedType*>& typeStack, const std::vector<std::string>& injectBinding = std::vector<std::string>());

ReflectedType* ProcessStruct(stb_lexer* lexer, bool isClass, ReflectionDatabase* database, bool defaultIsPublic, std::vector<ReflectedType*>& typeStack);

ReflectedType* ProcessEnum(stb_lexer* lexer, ReflectionDatabase* db);

void ProcessReflected(stb_lexer* lexer, ReflectionDatabase* database, std::vector<ReflectedType*>& typeStack);
