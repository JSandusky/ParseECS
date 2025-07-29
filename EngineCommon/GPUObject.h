#pragma once

#ifdef OENGINE_OPENGL
#else
    #include <d3d11.h>
#endif

struct GPUDevice;

struct GPUObject
{
    virtual bool Bind(GPUDevice* device, unsigned target = 0) = 0;
    virtual void Release() = 0;
};