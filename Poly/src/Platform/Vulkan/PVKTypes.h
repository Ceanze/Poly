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
        VERTEX                  = 0x00000001,
        TESSELLATION_CONTROL    = 0x00000002,
        TESSELLATION_EVALUATION = 0x00000004,
        GEOMETRY                = 0x00000008,
        FRAGMENT                = 0x00000010,
        COMPUTE                 = 0x00000020,
        ALL_GRAPHICS            = 0x0000001F,
        ALL                     = 0x7FFFFFFF,
        RAYGEN                  = 0x00000100,
        ANY_HIT                 = 0x00000200,
        CLOSEST_HIT             = 0x00000400,
        MISS                    = 0x00000800,
        INTERSECTION            = 0x00001000,
        CALLABLE                = 0x00002000,
        TASK                    = 0x00000040,
        MESH                    = 0x00000080
    };
    typedef ShaderStage ShaderType;
    ENABLE_BITMASK_OPERATORS(ShaderStage);

    enum class QueueType {
        GRAPHICS            = 0x00000001,
        COMPUTE             = 0x00000002,
        TRANSFER            = 0x00000004,
        SPARSE_BINDING      = 0x00000008,
        PROTECTED           = 0x00000010,
        FLAG_BITS_MAX_ENUM  = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(QueueType);

    enum class BufferType
    {
        SAMPLER                 = 0,
        COMBINED_IMAGE_SAMPLER  = 1,
        SAMPLED_IMAGE           = 2,
        STORAGE_IMAGE           = 3,
        UNIFORM_TEXEL           = 4,
        STORAGE_TEXEL           = 5,
        UNIFORM                 = 6,
        STORAGE                 = 7,
        UNIFORM_DYNAMIC         = 8,
        STORAGE_DYNAMIC         = 9,
        INPUT_ATTACHMENT        = 10,
    };

    enum class BufferUsage
    {
        TRANSFER_SRC                        = 0x00000001,
        TRANSFER_DST                        = 0x00000002,
        UNIFORM_TEXEL_BUFFER                = 0x00000004,
        STORAGE_TEXEL_BUFFER                = 0x00000008,
        UNIFORM_BUFFER                      = 0x00000010,
        STORAGE_BUFFER                      = 0x00000020,
        INDEX_BUFFER                        = 0x00000040,
        VERTEX_BUFFER                       = 0x00000080,
        INDIRECT_BUFFER                     = 0x00000100,
        TRANSFORM_FEEDBACK_BUFFER           = 0x00000800,
        TRANSFORM_FEEDBACK_COUNTER_BUFFER   = 0x00001000,
        CONDITIONAL_RENDERING               = 0x00000200,
        RAY_TRACING                         = 0x00000400,
        SHADER_DEVICE_ADDRESS               = 0x00020000,
        FLAG_BITS_MAX_ENUM                  = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(BufferUsage);

    enum class ImageUsage
    {
        TRANSFER_SRC            = 0x00000001,
        TRANSFER_DST            = 0x00000002,
        SAMPLED                 = 0x00000004,
        STORAGE                 = 0x00000008,
        COLOR_ATTACHMENT        = 0x00000010,
        DEPTH_STENCIL_ATTACHMEN = 0x00000020,
        TRANSIENT_ATTACHMENT    = 0x00000040,
        INPUT_ATTACHMENT        = 0x00000080,
        SHADING_RATE_IMAGE      = 0x00000100,
        FRAGMENT_DENSITY_MAP    = 0x00000200,
        FLAG_BITS_MAX_ENUM      = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(ImageUsage);

    enum class ImageCreate
    {
        SPARSE_BINDING                      = 0x00000001,
        SPARSE_RESIDENCY                    = 0x00000002,
        SPARSE_ALIASED                      = 0x00000004,
        MUTABLE_FORMAT                      = 0x00000008,
        CUBE_COMPATIBLE                     = 0x00000010,
        ALIAS                               = 0x00000400,
        SPLIT_INSTANCE_BIND_REGIONS         = 0x00000040,
        ARRAY_2D_COMPATIBLE                 = 0x00000020,
        BLOCK_TEXEL_VIEW_COMPATIBLE         = 0x00000080,
        EXTENDED_USAGE                      = 0x00000100,
        PROTECTED                           = 0x00000800,
        DISJOINT                            = 0x00000200,
        CORNER_SAMPLED                      = 0x00002000,
        SAMPLE_LOCATIONS_COMPATIBLE_DEPTH   = 0x00001000,
        SUBSAMPLED                          = 0x00004000,
        FLAG_BITS_MAX_ENUM = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(ImageCreate);

    enum class ImageLayout
    {
        UNDEFINED                                   = 0,
        GENERAL                                     = 1,
        COLOR_ATTACHMENT_OPTIMAL                    = 2,
        DEPTH_STENCIL_ATTACHMENT_OPTIMAL            = 3,
        DEPTH_STENCIL_READ_ONLY_OPTIMAL             = 4,
        SHADER_READ_ONLY_OPTIMAL                    = 5,
        TRANSFER_SRC_OPTIMAL                        = 6,
        TRANSFER_DST_OPTIMAL                        = 7,
        PREINITIALIZED                              = 8,
        DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL  = 1000117000,
        DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL  = 1000117001,
        PRESENT_SRC_KHR                             = 1000001002,
        SHARED_PRESENT_KHR                          = 1000111000,
        SHADING_RATE_OPTIMAL_NV                     = 1000164003,
        FRAGMENT_DENSITY_MAP_OPTIMAL_EXT            = 1000218000,
        DEPTH_ATTACHMENT_OPTIMAL_KHR                = 1000241000,
        DEPTH_READ_ONLY_OPTIMAL_KHR                 = 1000241001,
        STENCIL_ATTACHMENT_OPTIMAL_KHR              = 1000241002,
        STENCIL_READ_ONLY_OPTIMAL_KHR               = 1000241003,
        BEGIN_RANGE                                 = UNDEFINED,
        END_RANGE                                   = PREINITIALIZED,
        RANGE_SIZE                                  = (PREINITIALIZED - UNDEFINED + 1),
        MAX_ENUM                                    = 0x7FFFFFFF
    };

    enum class MemoryPropery
    {
        DEVICE_LOCAL            = 0x00000001,
        HOST_VISIBLE            = 0x00000002,
        HOST_COHERENT           = 0x00000004,
        HOST_CACHED             = 0x00000008,
        LAZILY_ALLOCATED        = 0x00000010,
        PROTECTED               = 0x00000020,
        DEVICE_COHERENT_AMD     = 0x00000040,
        DEVICE_UNCACHED_AMD     = 0x00000080,
        HOST_VISIBLE_COHERENT   = HOST_VISIBLE | HOST_COHERENT,
        FLAG_BITS_MAX_ENUM      = 0x7FFFFFFF
    };
    ENABLE_BITMASK_OPERATORS(MemoryPropery);
}