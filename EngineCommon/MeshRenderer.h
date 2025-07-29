#pragma once

/// Handles the rendering, and automatic instancing/batching of meshes.
struct MeshRenderer
{

};

/// Does similar work as a mesh renderer but is instead able to render geometries with bone transforms or morph targets.
/// Bone transforms are not assumed to be singular, presently this is only used for passing 'previous' and 'current' bone transforms for calculating velocity for motion-blur/etc
struct AnimatedMeshRenderer
{

};