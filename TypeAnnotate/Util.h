#pragma once

#include "Types.h"

/// Used for making function comparisons to check for things like callback compatibility.
struct Signature
{
    TypeInstance returnType_;
    std::vector<TypeInstance> parameterTypes_;
};

/// Test if the signature (const&, etc) matches between 2 type instances.
bool SignatureMatches(const TypeInstance& lhs, const TypeInstance& rhs);
/// Test if a method/function's signature matches a reference signature.
bool SignatureMatches(Method* meth, const Signature&);
/// Construct a signature from a method.
Signature SignatureOf(Method* meth);

/// In some cases may make type construction easier
/// DEPRECATED: favor TypeInstanceFromString to construct TypeInstance from "const std::string&" instead of this
///     builder interface.
struct TypeInstanceBuilder
{
    TypeInstance type;

    TypeInstanceBuilder() { }
    TypeInstanceBuilder(TypeInstance t) : type(t) { }

    TypeInstanceBuilder& MakeConst();
    TypeInstanceBuilder& MakePointer();
    TypeInstanceBuilder& MakeReference();
    TypeInstanceBuilder& MakeType(const char* name, ReflectionDatabase*);
    TypeInstanceBuilder& AddTemplateParam(TypeInstance);


    /*
    TypeInstanceBuilder().MakeType("std::string", db)
        .MakeConst()
        .MakeReference()
        .type
    */

};