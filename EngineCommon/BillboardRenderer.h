#pragma once

#include <vector>

/// Manages a collection of simple billboard rendered objects.
struct BillboardRenderer
{
    struct BBVertex
    {
        float position;
        float uv;
        float color;
        unsigned bbID_;
        float time_;
    };
    
    struct BillboardBufferData
    {
        BBVertex vertices_[4];
    };

    /// Contains the data used for vertex buffer reconstruction
    struct BillboardInstanceData
    {
        float size_ = 1.0f;
        float uv_ = 1.0f;
        float color_;
        float fixedSizeRatio_ = 1.0f;
        unsigned bbID_ = 0;     // Unique identifier for the billboard
        float bbTime_ = 0.0f;   // time is accumulated for as long as the billboard lives
    };

    /// Contains a local copy of the vertex buffer data.
    std::vector<BillboardBufferData> billboardData_;
    /// Number of billboards that are actually used.
    size_t usedCount_;
    /// List of the instance data for each billboard, always kept in a collapsed form via swap & pop
    std::vector<BillboardInstanceData> billboardInstances_;
};