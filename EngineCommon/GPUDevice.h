#pragma once

#include "GPUContants.h"

#ifdef OENGINE_OPENGL
#else
    #include <d3d11.h>
#endif

struct GPUDevice
{
#ifdef OENGINE_OPENGL
#else
    ID3D11Device* device_ = 0x0;
    ID3D11DeviceContext* context_ = 0x0;
#endif    

    void SetPrimitiveType(GPUPrimitiveType type);

    void Release();
};