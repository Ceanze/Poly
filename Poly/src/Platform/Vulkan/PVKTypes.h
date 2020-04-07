#pragma once

#include "PVKQueue.h"
#include "Poly/Core/PolyUtils.h"
#include <type_traits>

/*
	All the types used by the Poly Vulkan implementation is defined in this file.
*/



namespace Poly
{
    enum class ShaderStage {
        VERTEX = 0x00000001,
        TESSELLATION_CONTROL = 0x00000002,
        TESSELLATION_EVALUATION = 0x00000004,
        GEOMETRY = 0x00000008,
        FRAGMENT = 0x00000010,
        COMPUTE = 0x00000020,
        ALL_GRAPHICS = 0x0000001F,
        ALL = 0x7FFFFFFF,
        RAYGEN = 0x00000100,
        ANY_HIT = 0x00000200,
        CLOSEST_HIT = 0x00000400,
        MISS = 0x00000800,
        INTERSECTION = 0x00001000,
        CALLABLE = 0x00002000,
        TASK = 0x00000040,
        MESH = 0x00000080
    };
    typedef ShaderStage ShaderType;
    ENABLE_BITMASK_OPERATORS(ShaderStage);

    enum class QueueType {
        GRAPHICS = 0x00000001,
        COMPUTE = 0x00000002,
        TRANSFER = 0x00000004,
        SPARSE_BINDING = 0x00000008,
        PROTECTED = 0x00000010,
        FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(QueueType);

    enum class BufferType
    {
        SAMPLER = 0,
        COMBINED_IMAGE_SAMPLER = 1,
        SAMPLED_IMAGE = 2,
        STORAGE_IMAGE = 3,
        UNIFORM_TEXEL = 4,
        STORAGE_TEXEL = 5,
        UNIFORM = 6,
        STORAGE = 7,
        UNIFORM_DYNAMIC = 8,
        STORAGE_DYNAMIC = 9,
        INPUT_ATTACHMENT = 10,
    };
    ENABLE_BITMASK_OPERATORS(BufferType);

    enum class BufferUsage
    {
        TRANSFER_SRC = 0x00000001,
        TRANSFER_DST = 0x00000002,
        UNIFORM_TEXEL_BUFFER = 0x00000004,
        STORAGE_TEXEL_BUFFER = 0x00000008,
        UNIFORM_BUFFER = 0x00000010,
        STORAGE_BUFFER = 0x00000020,
        INDEX_BUFFER = 0x00000040,
        VERTEX_BUFFER = 0x00000080,
        INDIRECT_BUFFER = 0x00000100,
        TRANSFORM_FEEDBACK_BUFFER = 0x00000800,
        TRANSFORM_FEEDBACK_COUNTER_BUFFER = 0x00001000,
        CONDITIONAL_RENDERING = 0x00000200,
        RAY_TRACING = 0x00000400,
        SHADER_DEVICE_ADDRESS = 0x00020000,
        FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(BufferUsage);

    enum class MemoryPropery
    {
        DEVICE_LOCAL = 0x00000001,
        HOST_VISIBLE = 0x00000002,
        HOST_COHERENT = 0x00000004,
        HOST_CACHED = 0x00000008,
        LAZILY_ALLOCATED = 0x00000010,
        PROTECTED = 0x00000020,
        DEVICE_COHERENT_AMD = 0x00000040,
        DEVICE_UNCACHED_AMD = 0x00000080,
        HOST_VISIBLE_COHERENT = HOST_VISIBLE | HOST_COHERENT,
        FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(MemoryPropery);
}