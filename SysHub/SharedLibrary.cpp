#include "SharedLibrary.h"

SharedLibrary::SharedLibrary(const char* path) :
#ifdef WIN32
    module_(0)
#else
#endif
{
    if (path)
    {
#ifdef WIN32
        if (module_ = LoadLibraryA(path))
        {
            //TODO: log library load
        }
        else
        {
            //TODO: log failure
            module_ = 0;
        }
#endif
    }
}

SharedLibrary::~SharedLibrary()
{
#ifdef WIN32
    if (module_)
    {
        if (!FreeLibrary(module_))
        {
            //TODO: log error
        }
        module_ = 0;
    }
#endif
}

bool SharedLibrary::IsLoaded() const
{
#ifdef WIN32
    return module_ != 0;
#endif
}

void* SharedLibrary::GetFunction(const char* functionName)
{
#ifdef WIN32
    if (module_)
        return GetProcAddress(module_, functionName);
#endif
    return 0x0;
}