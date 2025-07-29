#include "GPUBuffer.h"

#include "GPUDevice.h"

namespace OEngine
{

    bool GPUBuffer::Bind(GPUDevice* device, unsigned target)
    {
        if (buffer_ && bufferType_ != GPUBuffer_Invalid)
        {
            switch (bufferType_)
            {
            case GPUBuffer_Vertex:
                device->context_->IASetVertexBuffers(target, 1, &buffer_, 0, 0);
                break;
            case GPUBuffer_Index:
                device->context_->IASetIndexBuffer(buffer_, DXGI_FORMAT::DXGI_FORMAT_R32_UINT, 0);
                break;
            case GPUBuffer_Instance:
                device->context_->IASetVertexBuffers(target + 1, 1, &buffer_, 0, 0);
                break;
            case GPUBuffer_Constant:
                device->context_->PSSetConstantBuffers(target, 1, &buffer_);
                break;
            }
        }
        return false;
    }

    void GPUBuffer::Release()
    {
        if (srv_)
            srv_->Release();
        srv_ = 0x0;

        if (uav_)
            uav_->Release();
        uav_ = 0x0;

        if (buffer_)
        {
            ID3D11Device* device = 0x0;
            buffer_->GetDevice(&device);
            if (device)
            {
                buffer_->Release();
                buffer_ = 0x0;
            }
        }
    }

    GPUBuffer::~GPUBuffer()
    {
        Release();
    }

    bool GPUTexture::Bind(GPUDevice* device, unsigned target)
    {
        if (oneD_ && textureType_ != Texture_Invalid && srv_)
        {
            device->context_->PSSetShaderResources(target, 1, &srv_);
            return true;
        }
        return false;
    }

    void GPUTexture::Release()
    {
        if (oneD_ && textureType_ != Texture_Invalid)
        {
            switch (textureType_)
            {
            case Texture_1D:
                oneD_->Release();
                break;
            case Texture_2D:
            case Texture_Array:
            case Texture_Cube:
            case Texture_CubeArray:
                twoD_->Release();
                break;
            case Texture_3D:
                threeD_->Release();
                break;
            }

            oneD_ = 0x0;
            textureType_ = Texture_Invalid;
        }
        GPUBuffer::Release();
    }

    void GPUTexture::GenerateMipMaps(bool force)
    {

    }

    GPUTexture::~GPUTexture()
    {
        Release();
    }
}