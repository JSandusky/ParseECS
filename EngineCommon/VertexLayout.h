#pragma once

#include "GPUObject.h"

#include <vector>

struct VertexLayout : public GPUObject
{
#ifdef OENGINE_OPENGL
#else
    ID3D11InputLayout* layout_ = 0x0;
    std::vector<D3D11_INPUT_ELEMENT_DESC> elements_;
#endif

    virtual void Release() override;
};