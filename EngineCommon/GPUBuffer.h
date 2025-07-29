#pragma once

#include "GPUObject.h"

namespace OEngine
{
    enum GPUBufferType
    {
        GPUBuffer_Invalid,
        GPUBuffer_Vertex,
        GPUBuffer_Index,
        GPUBuffer_Instance,
        GPUBuffer_Constant,
        GPUBuffer_Texture
    };

    enum TextureType
    {
        Texture_Invalid,
        Texture_1D,
        Texture_2D,
        Texture_3D,
        Texture_Cube,
        Texture_Array,
        Texture_CubeArray
    };

    struct GPUBuffer : public GPUObject
    {
        GPUBufferType bufferType_ = GPUBuffer_Invalid;
        #ifdef OENGINE_OPENGL
        #else
            ID3D11Buffer* buffer_;
            ID3D11ShaderResourceView* srv_;
            ID3D11UnorderedAccessView* uav_;
        #endif

        virtual bool Bind(GPUDevice* device, unsigned target = 0) override;
        virtual void Release() override;
        virtual ~GPUBuffer();
    };

    struct GPUTexture : public GPUBuffer
    {
        TextureType textureType_ = Texture_Invalid;
#ifdef OENGINE_OPENL
#else
        union {
            ID3D11Texture1D* oneD_;
            ID3D11Texture2D* twoD_;
            ID3D11Texture3D* threeD_;
        };
#endif
        bool fullyMipped_ = false;

        virtual bool Bind(GPUDevice* device, unsigned target = 0) override;
        virtual void Release() override;

        /// Reads the data from the GPU object back into memory
        //void ReadImage(FilterableBlockmap<RGBA8>* intoImage, unsigned index, unsigned mip) const;
        /// Writes data into the GPU object
        //void WriteImage(FilterableBlockmap<RGBA8>* fromImage, unsigned index, unsigned mip) const;

        /// Generates mipmaps for the texture(s), if not set to *force* then it will only mip if *fullyMipped_* is not true
        void GenerateMipMaps(bool force = false);

        virtual ~GPUTexture();
    };
}