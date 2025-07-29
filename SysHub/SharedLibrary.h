#pragma once

#include "SysDef.h"
#include "Platform.h"

/// Wraps access to DLL/shared-libraries.
/// Although we shall assume that CRT MT-DLL (or MTD-DLL) is used, for 'nix/mac there shall be no trust.
class SYS_EXPORT SharedLibrary
{
public:
    /// Construct a shared library from the given path.
    SharedLibrary(const char* path);
    /// Destruct and unload the library if it had been successfully loaded.
    virtual ~SharedLibrary();

    /// Returns true if the library was successfully loaded.
    bool IsLoaded() const;
    /// Returns a pointer to the given function or null.
    void* GetFunction(const char* functionName);

private:
#ifdef WIN32
    /// Win32 DLL module for proc functions.
    HMODULE module_;
#endif
};