#pragma once

#include <vector>

enum RenderStageCommand
{
    RSC_Invalid = 0,
    RSC_Clear,              // Clear buffers
    RSC_ScreenQuad,         // Draw a screenspace-screen quad
    RSC_Geometry,           // Perform a geometry pass
    RSC_BindTarget,         // Bind a rendertarget
    RSC_DeferredVolumes,    // Perform a deferred volumes pass
    RSC_Compute,            // Run a compute shader
    RSC_Custom,             // Custom stage
};

struct RenderStage
{
    RenderStageCommand command_;
};

struct RenderPipeline
{
    std::vector<RenderStage> stages_;
};