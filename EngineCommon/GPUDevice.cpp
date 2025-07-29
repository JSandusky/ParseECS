#include "GPUDevice.h"

void GPUDevice::SetPrimitiveType(GPUPrimitiveType type)
{
    if (context_)
    {
        switch (type)
        {
        case Prim_Points:
            context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_POINTLIST);
            break;
        case Prim_LineList:
            context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST);
            break;
        case Prim_LineStrip:
            context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINESTRIP);
            break;
        case Prim_TriangleList:
            context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            break;
        case Prim_TriangleStrip:
            context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
            break;
        case Prim_LineList_Adjacent:
            context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_LINELIST_ADJ);
            break;
        case Prim_TriangleList_Adjacent:
            context_->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST_ADJ);
            break;
        }
    }
}

void GPUDevice::Release()
{
    if (context_ && device_)
    {
        context_->Release();
        device_->Release();
    }
    context_ = 0x0;
    device_ = 0x0;
}