#pragma once

#include <cstdint>

#ifdef SYS_HUB_EXPORT
    #define SYS_EXPORT __declspec(dllexport)
#else
    #define SYS_EXPORT
#endif

#define Kilobytes(VALUE) (VALUE * 1024)
#define Megabytes(VALUE) (Kilobytes(VALUE) * 1024)
#define Gigabytes(VALUE) (Megabytes((uint64_t)VALUE) * 1024)