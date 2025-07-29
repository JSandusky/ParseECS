#include "GPUShaderProgram.h"

bool GPUShaderProgram::Bind(GPUDevice* device, unsigned target)
{
#define SHADER_BIND(SHADER) if (SHADER) SHADER->Bind(device, target);

    SHADER_BIND(vertexShader_);
    SHADER_BIND(hullShader_);
    SHADER_BIND(domainShader_);
    SHADER_BIND(geometryShader_);
    SHADER_BIND(pixelShader_);

#undef SHADER_BIND
    return false;
}

void GPUShaderProgram::Release()
{
#define SHADER_RELEASE(SHADER) if (SHADER) SHADER->Release();

    SHADER_RELEASE(vertexShader_);
    SHADER_RELEASE(hullShader_);
    SHADER_RELEASE(domainShader_);
    SHADER_RELEASE(geometryShader_);
    SHADER_RELEASE(pixelShader_);

#undef SHADER_RELEASE;
}

VertexLayout* GPUShaderProgram::GetVertexLayout()
{
    return 0x0;
}