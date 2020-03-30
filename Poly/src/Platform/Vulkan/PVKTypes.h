#pragma once

#include "PVKQueue.h"

/*
	All the types used by the Poly Vulkan implementation is defined in this file.
*/

namespace Poly
{
	enum class ShaderType {
        VERTEX_BIT = 0x00000001,
        TESSELLATION_CONTROL_BIT = 0x00000002,
        TESSELLATION_EVALUATION_BIT = 0x00000004,
        GEOMETRY_BIT = 0x00000008,
        FRAGMENT_BIT = 0x00000010,
        COMPUTE_BIT = 0x00000020,
        ALL_GRAPHICS = 0x0000001F,
        ALL = 0x7FFFFFFF,
        RAYGEN_BIT_NV = 0x00000100,
        ANY_HIT_BIT_NV = 0x00000200,
        CLOSEST_HIT_BIT_NV = 0x00000400,
        MISS_BIT_NV = 0x00000800,
        INTERSECTION_BIT_NV = 0x00001000,
        CALLABLE_BIT_NV = 0x00002000,
        TASK_BIT_NV = 0x00000040,
        MESH_BIT_NV = 0x00000080
	};

    enum class QueueType {
        GRAPHICS_BIT = 0x00000001,
        COMPUTE_BIT = 0x00000002,
        TRANSFER_BIT = 0x00000004,
        SPARSE_BINDING_BIT = 0x00000008,
        PROTECTED_BIT = 0x00000010,
        FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
}