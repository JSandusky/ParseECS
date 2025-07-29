#pragma once

#include <string>

class ReflectedType;
class ReflectionDatabase;

std::string PrintCode(ReflectedType* type);
std::string PrintCalls(ReflectedType* type);
std::string PrintImgui(ReflectedType* type, ReflectionDatabase* database);
std::string GenerateFunctionDefs(ReflectionDatabase* db, const std::string& fwd);