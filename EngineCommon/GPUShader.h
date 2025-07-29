#pragma once

#include "GPUObject.h"

enum ShaderType
{
    Shader_Invalid,
    Shader_Vertex,
    Shader_Domain,
    Shader_Hull,
    Shader_Geometry,
    Shader_Pixel,
    Shader_Compute
};

/// A GPUShader is the shader for a single stage of the rendering pipeline.
struct GPUShader : public GPUObject
{
    ShaderType shaderType_ = Shader_Invalid;
#ifdef OENGINE_OPENGL
#else
    union {
        ID3D11VertexShader* vertexShader_;
        ID3D11GeometryShader* geometryShader_;
        ID3D11PixelShader* pixelShader_;
        ID3D11DomainShader* domainShader_;
        ID3D11HullShader* hullShader_;
        ID3D11ComputeShader* computeShader_;
    };
#endif

    GPUShader();

    virtual bool Bind(GPUDevice* device, unsigned target = 0) override;
    virtual void Release() override;
};