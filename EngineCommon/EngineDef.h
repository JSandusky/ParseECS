#pragma once

#ifndef ENGINE_EXPORTS
    #define ENGINE_DLL __declspec(dllexport)
#else
    #define ENGINE_DLL __declspec(dllimport)
#endif