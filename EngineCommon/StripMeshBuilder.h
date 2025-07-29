#pragma once

/// Builds a quad-strip mesh, takes a series of points and texture parameters to produce a nice strip.
struct StripMeshBuilder
{
    /// Generate a triangle at the head of the strip.
    bool openingTriangle_;
    /// Generate a triangle and the tail of the strip.
    bool tailingTriangle_;
    /// Will write adjacency information (either agressively into the vertex data for legacy or via outputting as a TriangleList_Adjacency
    bool writeAdjacenty_;
};

/// More intensive builder that can intelligently perform texture substitution of each quad.
struct SmartStripMeshBuilder
{

};

/// Triangulates a list of points for rendering and can optionally generate UV coordinates for it. Does not need to be convex, but cannot contain holes.
struct PolygonMeshBuilder
{

};

/// Triangulates a convex 3d shape and can optionally produce UV coordinates and other vertex attributes for it.
struct ConvexMeshBuilder
{

};