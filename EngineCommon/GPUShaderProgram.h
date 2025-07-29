#pragma once

#include "GPUShader.h"

struct VertexLayout;

/// A GPUShaderProgram is a complete collection of shader stages for the entire rendering pipeline
struct GPUShaderProgram : public GPUObject
{
    GPUShader* vertexShader_ = 0x0;
    GPUShader* hullShader_ = 0x0;
    GPUShader* domainShader_ = 0x0;
    GPUShader* geometryShader_ = 0x0;
    GPUShader* pixelShader_ = 0x0;

    /// Binds all of the shaders into the device context.
    virtual bool Bind(GPUDevice* device, unsigned target = 0) override;
    /// Releases all of the contained shaders, does not free the GPUShader objects.
    virtual void Release() override;

    /// Gets the vertex layout for this shader.
    VertexLayout* GetVertexLayout();
};