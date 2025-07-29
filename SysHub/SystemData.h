#pragma once

#include "SysDef.h"

struct MemoryManager;

SYS_EXPORT struct SystemInformation
{
    uint32_t windowHandle_;
    MemoryManager* memory_;
};