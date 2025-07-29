#pragma once

#include <vector>

typedef float float3;
typedef float float2;
typedef float float4;
typedef unsigned gpuColor;

/// Renders geometry that is only a set of world-space vertices and colors.
/// Index buffers are not used for any of the rendering.
struct DebugGeometry
{
    /// Position and color pair for debug geometry.
    struct DebugVertex
    {
        float3 position;
        gpuColor color;
    };

    /// Debug triangles are rendered are Triangle lists
    struct DebugTriangle
    {
        DebugVertex vertices_[3];
    };

    std::vector<DebugVertex> lines_;
    std::vector<DebugTriangle> triangles_;
    /// Centroids of all triangles for sorting.
    std::vector<float3> triangleCentroids_;

// All other functions makes calls to these functions
    /// Adds a single color line to the list.
    void AddLine(float3 a, float3 b, gpuColor color);
    /// Adds a two color line to the list.
    void AddLine(float3 a, float3 b, gpuColor colorA, gpuColor colorB);
    /// Adds a single color triangle to the list.
    void AddTriangle(float3 a, float3 b, float3 c, gpuColor color);
    /// Adds a triangle with different colors for all vertices to the list.
    void AddTriangle(float3 a, float3 b, float3 c, gpuColor colorA, gpuColor colorB, gpuColor colorC);
    /// Adds two triangles forming a quad.
    void AddQuad(float3 a, float3 b, float3 c, float3 d, gpuColor color);

    /// Adds an XYZ transform axis indicator
    void AddTransform(float3 center, float radius);
    /// Adds a 3d sphere
    void AddSphere(float3 center, float radius, gpuColor color);
    /// Adds a cylinder
    void AddCylinder(float3 center, float height, float radius, gpuColor color);
    /// Adds a cross.
    void AddCross(float3 center, float radius, gpuColor color);
    /// Adds a bounding box
    void AddBounds(gpuColor color);
    /// Add a circle on a plane
    void AddCircle(float3 center, float radius, float3 normal, gpuColor color);
    /// Add a cone.
    void AddCone(float3 tip, float height, float radius, gpuColor color);
    /// Add a capsule shape
    void AddCapsule(float3 center, float height, float radius, gpuColor color);
    /// Add a convex hull
    void AddConvex(gpuColor color);
    /// Adds a camera view frustrum
    void AddFrustum(gpuColor color);
    /// Adds a geometry mesh as triangles.
    void AddMesh(gpuColor color);
    /// Adds a geometry mesh as a wireframe of lines.
    void AddMeshWire(gpuColor color);
    /// Adds a skeletal object
    void AddSkeleton(gpuColor color);
};

/// Contains a few DebugGeometry objects for rendering multi-functional debug drawing. 
/// Depth-tested objects are draw last (as greater-equal) so that the non-depthtested versions can be drawn for facing
struct DebugRenderer
{
    /// This geometry will be drawn without depth testing
    DebugGeometry notDepthTested_;
    /// This geometry will be drawn with depth testing
    DebugGeometry depthTested_;
};