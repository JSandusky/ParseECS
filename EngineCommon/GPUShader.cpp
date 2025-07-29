#include "GPUShader.h"

#include "GPUDevice.h"

GPUShader::GPUShader()
{
    /// type punned create
    vertexShader_ = 0x0;
}

bool GPUShader::Bind(GPUDevice* device, unsigned target)
{
    // type-punned null check and validity check
    if (vertexShader_ && shaderType_ != Shader_Invalid)
    {
        switch (shaderType_)
        {
        case Shader_Vertex:
            device->context_->VSSetShader(vertexShader_, 0x0, 0);
            break;
        case Shader_Pixel:
            device->context_->PSSetShader(pixelShader_, 0x0, 0);
            break;
        case Shader_Geometry:
            device->context_->GSSetShader(geometryShader_, 0x0, 0);
            break;
        case Shader_Hull:
            device->context_->HSSetShader(hullShader_, 0x0, 0);
            break;
        case Shader_Domain:
            device->context_->DSSetShader(domainShader_, 0x0, 0);
            break;
        case Shader_Compute:
            device->context_->CSSetShader(computeShader_, 0x0, 0);
            break;
        }
        return true;
    }
    return false;
}
void GPUShader::Release()
{
    if (vertexShader_ && shaderType_ != Shader_Invalid)
    {
        switch (shaderType_)
        {
        case Shader_Vertex:
            vertexShader_->Release();
            break;
        case Shader_Pixel:
            pixelShader_->Release();
            break;
        case Shader_Geometry:
            geometryShader_->Release();
            break;
        case Shader_Hull:
            hullShader_->Release();
            break;
        case Shader_Domain:
            domainShader_->Release();
            break;
        case Shader_Compute:
            computeShader_->Release();
            break;
        }
    }
    // type punned clear
    vertexShader_ = 0x0;
    shaderType_ = Shader_Invalid;
}