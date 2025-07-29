#pragma once



    enum GPUPrimitiveType
    {
        Prim_Invalid = 0,
        Prim_Points,
        Prim_LineList,
        Prim_LineStrip,
        Prim_TriangleList,
        Prim_TriangleStrip,
        Prim_TriangleList_Adjacent, // For things that need geometry shaders with adjacency information
        Prim_LineList_Adjacent,     // For things that need geometry shaders with adjacency information
    };
