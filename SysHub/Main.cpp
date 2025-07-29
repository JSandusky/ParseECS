#include "SysDef.h"

#include "Platform.h"

#include "Buffer.h"

#ifdef WIN32
BOOL WINAPI DllMain(_In_ HINSTANCE hinstDLL, _In_ DWORD     fdwReason, _In_ LPVOID    lpvReserved)
{
    switch (fdwReason)
    {
    case DLL_PROCESS_ATTACH:
        break;
    case DLL_PROCESS_DETACH:
        break;
    case DLL_THREAD_ATTACH:
        break;
    case DLL_THREAD_DETACH:
        break;
    }

    float fl[] = { 5, 3, 2 };
    Span<float> span(fl, 3);
    for (auto sp : span)
    {

    }
    return TRUE;
}
#endif