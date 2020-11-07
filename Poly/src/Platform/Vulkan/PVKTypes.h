#pragma once

#include <vulkan/vulkan.h>
#include "Poly/Core/PolyUtils.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include <type_traits>
#include "PVKQueue.h"

/*
	All the types used by the Poly Vulkan implementation is defined in this file.
*/

/* TODO:
	REWORK IN PROGRESS: This file is misguiding, these types are common across all of the platforms, not just vulkan.
	Therefore this file will change purpose to convert the common types to the correct vulkan types instead.
	This makes sure that the common types are API independent (which the current in this file isn't)
*/


namespace Poly
{
	inline VkShaderStageFlagBits ConvertShaderStageVK(ShaderStage stage)
	{
		switch (stage)
		{
		case ShaderStage::VERTEX:					return VK_SHADER_STAGE_VERTEX_BIT;
		case ShaderStage::FRAGMENT:					return VK_SHADER_STAGE_FRAGMENT_BIT;
		case ShaderStage::COMPUTE:					return VK_SHADER_STAGE_COMPUTE_BIT;
		case ShaderStage::TESSELLATION_CONTROL:		return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case ShaderStage::TESSELLATION_EVALUATION:	return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case ShaderStage::GEOMETRY:					return VK_SHADER_STAGE_GEOMETRY_BIT;
		case ShaderStage::RAYGEN:					return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case ShaderStage::ANY_HIT:					return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case ShaderStage::CLOSEST_HIT:				return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case ShaderStage::MISS:						return VK_SHADER_STAGE_MISS_BIT_KHR;
		case ShaderStage::INTERSECTION:				return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case ShaderStage::CALLABLE:					return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		case ShaderStage::TASK:						return VK_SHADER_STAGE_TASK_BIT_NV;
		case ShaderStage::MESH:						return VK_SHADER_STAGE_MESH_BIT_NV;
		default:									return VK_SHADER_STAGE_ALL;
		}
	}

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
		TRANSFER_SRC                = 0x00000001,
		TRANSFER_DST                = 0x00000002,
		SAMPLED                     = 0x00000004,
		STORAGE                     = 0x00000008,
		COLOR_ATTACHMENT            = 0x00000010,
		DEPTH_STENCIL_ATTACHMENT    = 0x00000020,
		TRANSIENT_ATTACHMENT        = 0x00000040,
		INPUT_ATTACHMENT            = 0x00000080,
		SHADING_RATE_IMAGE          = 0x00000100,
		FRAGMENT_DENSITY_MAP        = 0x00000200,
		FLAG_BITS_MAX_ENUM          = 0x7FFFFFFF
	};
	ENABLE_BITMASK_OPERATORS(ImageUsage);

	enum class ImageCreate
	{
		NONE                                = 0x00000000,
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

	enum class ColorFormat
	{
		UNDEFINED                                   = 0,
		R4G4_UNORM_PACK8                            = 1,
		R4G4B4A4_UNORM_PACK16                       = 2,
		B4G4R4A4_UNORM_PACK16                       = 3,
		R5G6B5_UNORM_PACK16                         = 4,
		B5G6R5_UNORM_PACK16                         = 5,
		R5G5B5A1_UNORM_PACK16                       = 6,
		B5G5R5A1_UNORM_PACK16                       = 7,
		A1R5G5B5_UNORM_PACK16                       = 8,
		R8_UNORM                                    = 9,
		R8_SNORM                                    = 10,
		R8_USCALED                                  = 11,
		R8_SSCALED                                  = 12,
		R8_UINT                                     = 13,
		R8_SINT                                     = 14,
		R8_SRGB                                     = 15,
		R8G8_UNORM                                  = 16,
		R8G8_SNORM                                  = 17,
		R8G8_USCALED                                = 18,
		R8G8_SSCALED                                = 19,
		R8G8_UINT                                   = 20,
		R8G8_SINT                                   = 21,
		R8G8_SRGB                                   = 22,
		R8G8B8_UNORM                                = 23,
		R8G8B8_SNORM                                = 24,
		R8G8B8_USCALED                              = 25,
		R8G8B8_SSCALED                              = 26,
		R8G8B8_UINT                                 = 27,
		R8G8B8_SINT                                 = 28,
		R8G8B8_SRGB                                 = 29,
		B8G8R8_UNORM                                = 30,
		B8G8R8_SNORM                                = 31,
		B8G8R8_USCALED                              = 32,
		B8G8R8_SSCALED                              = 33,
		B8G8R8_UINT                                 = 34,
		B8G8R8_SINT                                 = 35,
		B8G8R8_SRGB                                 = 36,
		R8G8B8A8_UNORM                              = 37,
		R8G8B8A8_SNORM                              = 38,
		R8G8B8A8_USCALED                            = 39,
		R8G8B8A8_SSCALED                            = 40,
		R8G8B8A8_UINT                               = 41,
		R8G8B8A8_SINT                               = 42,
		R8G8B8A8_SRGB                               = 43,
		B8G8R8A8_UNORM                              = 44,
		B8G8R8A8_SNORM                              = 45,
		B8G8R8A8_USCALED                            = 46,
		B8G8R8A8_SSCALED                            = 47,
		B8G8R8A8_UINT                               = 48,
		B8G8R8A8_SINT                               = 49,
		B8G8R8A8_SRGB                               = 50,
		A8B8G8R8_UNORM_PACK32                       = 51,
		A8B8G8R8_SNORM_PACK32                       = 52,
		A8B8G8R8_USCALED_PACK32                     = 53,
		A8B8G8R8_SSCALED_PACK32                     = 54,
		A8B8G8R8_UINT_PACK32                        = 55,
		A8B8G8R8_SINT_PACK32                        = 56,
		A8B8G8R8_SRGB_PACK32                        = 57,
		A2R10G10B10_UNORM_PACK32                    = 58,
		A2R10G10B10_SNORM_PACK32                    = 59,
		A2R10G10B10_USCALED_PACK32                  = 60,
		A2R10G10B10_SSCALED_PACK32                  = 61,
		A2R10G10B10_UINT_PACK32                     = 62,
		A2R10G10B10_SINT_PACK32                     = 63,
		A2B10G10R10_UNORM_PACK32                    = 64,
		A2B10G10R10_SNORM_PACK32                    = 65,
		A2B10G10R10_USCALED_PACK32                  = 66,
		A2B10G10R10_SSCALED_PACK32                  = 67,
		A2B10G10R10_UINT_PACK32                     = 68,
		A2B10G10R10_SINT_PACK32                     = 69,
		R16_UNORM                                   = 70,
		R16_SNORM                                   = 71,
		R16_USCALED                                 = 72,
		R16_SSCALED                                 = 73,
		R16_UINT                                    = 74,
		R16_SINT                                    = 75,
		R16_SFLOAT                                  = 76,
		R16G16_UNORM                                = 77,
		R16G16_SNORM                                = 78,
		R16G16_USCALED                              = 79,
		R16G16_SSCALED                              = 80,
		R16G16_UINT                                 = 81,
		R16G16_SINT                                 = 82,
		R16G16_SFLOAT                               = 83,
		R16G16B16_UNORM                             = 84,
		R16G16B16_SNORM                             = 85,
		R16G16B16_USCALED                           = 86,
		R16G16B16_SSCALED                           = 87,
		R16G16B16_UINT                              = 88,
		R16G16B16_SINT                              = 89,
		R16G16B16_SFLOAT                            = 90,
		R16G16B16A16_UNORM                          = 91,
		R16G16B16A16_SNORM                          = 92,
		R16G16B16A16_USCALED                        = 93,
		R16G16B16A16_SSCALED                        = 94,
		R16G16B16A16_UINT                           = 95,
		R16G16B16A16_SINT                           = 96,
		R16G16B16A16_SFLOAT                         = 97,
		R32_UINT                                    = 98,
		R32_SINT                                    = 99,
		R32_SFLOAT                                  = 100,
		R32G32_UINT                                 = 101,
		R32G32_SINT                                 = 102,
		R32G32_SFLOAT                               = 103,
		R32G32B32_UINT                              = 104,
		R32G32B32_SINT                              = 105,
		R32G32B32_SFLOAT                            = 106,
		R32G32B32A32_UINT                           = 107,
		R32G32B32A32_SINT                           = 108,
		R32G32B32A32_SFLOAT                         = 109,
		R64_UINT                                    = 110,
		R64_SINT                                    = 111,
		R64_SFLOAT                                  = 112,
		R64G64_UINT                                 = 113,
		R64G64_SINT                                 = 114,
		R64G64_SFLOAT                               = 115,
		R64G64B64_UINT                              = 116,
		R64G64B64_SINT                              = 117,
		R64G64B64_SFLOAT                            = 118,
		R64G64B64A64_UINT                           = 119,
		R64G64B64A64_SINT                           = 120,
		R64G64B64A64_SFLOAT                         = 121,
		B10G11R11_UFLOAT_PACK32                     = 122,
		E5B9G9R9_UFLOAT_PACK32                      = 123,
		D16_UNORM                                   = 124,
		X8_D24_UNORM_PACK32                         = 125,
		D32_SFLOAT                                  = 126,
		S8_UINT                                     = 127,
		D16_UNORM_S8_UINT                           = 128,
		D24_UNORM_S8_UINT                           = 129,
		D32_SFLOAT_S8_UINT                          = 130,
		BC1_RGB_UNORM_BLOCK                         = 131,
		BC1_RGB_SRGB_BLOCK                          = 132,
		BC1_RGBA_UNORM_BLOCK                        = 133,
		BC1_RGBA_SRGB_BLOCK                         = 134,
		BC2_UNORM_BLOCK                             = 135,
		BC2_SRGB_BLOCK                              = 136,
		BC3_UNORM_BLOCK                             = 137,
		BC3_SRGB_BLOCK                              = 138,
		BC4_UNORM_BLOCK                             = 139,
		BC4_SNORM_BLOCK                             = 140,
		BC5_UNORM_BLOCK                             = 141,
		BC5_SNORM_BLOCK                             = 142,
		BC6H_UFLOAT_BLOCK                           = 143,
		BC6H_SFLOAT_BLOCK                           = 144,
		BC7_UNORM_BLOCK                             = 145,
		BC7_SRGB_BLOCK                              = 146,
		ETC2_R8G8B8_UNORM_BLOCK                     = 147,
		ETC2_R8G8B8_SRGB_BLOCK                      = 148,
		ETC2_R8G8B8A1_UNORM_BLOCK                   = 149,
		ETC2_R8G8B8A1_SRGB_BLOCK                    = 150,
		ETC2_R8G8B8A8_UNORM_BLOCK                   = 151,
		ETC2_R8G8B8A8_SRGB_BLOCK                    = 152,
		EAC_R11_UNORM_BLOCK                         = 153,
		EAC_R11_SNORM_BLOCK                         = 154,
		EAC_R11G11_UNORM_BLOCK                      = 155,
		EAC_R11G11_SNORM_BLOCK                      = 156,
		ASTC_4x4_UNORM_BLOCK                        = 157,
		ASTC_4x4_SRGB_BLOCK                         = 158,
		ASTC_5x4_UNORM_BLOCK                        = 159,
		ASTC_5x4_SRGB_BLOCK                         = 160,
		ASTC_5x5_UNORM_BLOCK                        = 161,
		ASTC_5x5_SRGB_BLOCK                         = 162,
		ASTC_6x5_UNORM_BLOCK                        = 163,
		ASTC_6x5_SRGB_BLOCK                         = 164,
		ASTC_6x6_UNORM_BLOCK                        = 165,
		ASTC_6x6_SRGB_BLOCK                         = 166,
		ASTC_8x5_UNORM_BLOCK                        = 167,
		ASTC_8x5_SRGB_BLOCK                         = 168,
		ASTC_8x6_UNORM_BLOCK                        = 169,
		ASTC_8x6_SRGB_BLOCK                         = 170,
		ASTC_8x8_UNORM_BLOCK                        = 171,
		ASTC_8x8_SRGB_BLOCK                         = 172,
		ASTC_10x5_UNORM_BLOCK                       = 173,
		ASTC_10x5_SRGB_BLOCK                        = 174,
		ASTC_10x6_UNORM_BLOCK                       = 175,
		ASTC_10x6_SRGB_BLOCK                        = 176,
		ASTC_10x8_UNORM_BLOCK                       = 177,
		ASTC_10x8_SRGB_BLOCK                        = 178,
		ASTC_10x10_UNORM_BLOCK                      = 179,
		ASTC_10x10_SRGB_BLOCK                       = 180,
		ASTC_12x10_UNORM_BLOCK                      = 181,
		ASTC_12x10_SRGB_BLOCK                       = 182,
		ASTC_12x12_UNORM_BLOCK                      = 183,
		ASTC_12x12_SRGB_BLOCK                       = 184,
		G8B8G8R8_422_UNORM                          = 1000156000,
		B8G8R8G8_422_UNORM                          = 1000156001,
		G8_B8_R8_3PLANE_420_UNORM                   = 1000156002,
		G8_B8R8_2PLANE_420_UNORM                    = 1000156003,
		G8_B8_R8_3PLANE_422_UNORM                   = 1000156004,
		G8_B8R8_2PLANE_422_UNORM                    = 1000156005,
		G8_B8_R8_3PLANE_444_UNORM                   = 1000156006,
		R10X6_UNORM_PACK16                          = 1000156007,
		R10X6G10X6_UNORM_2PACK16                    = 1000156008,
		R10X6G10X6B10X6A10X6_UNORM_4PACK16          = 1000156009,
		G10X6B10X6G10X6R10X6_422_UNORM_4PACK16      = 1000156010,
		B10X6G10X6R10X6G10X6_422_UNORM_4PACK16      = 1000156011,
		G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16  = 1000156012,
		G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16   = 1000156013,
		G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16  = 1000156014,
		G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16   = 1000156015,
		G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16  = 1000156016,
		R12X4_UNORM_PACK16                          = 1000156017,
		R12X4G12X4_UNORM_2PACK16                    = 1000156018,
		R12X4G12X4B12X4A12X4_UNORM_4PACK16          = 1000156019,
		G12X4B12X4G12X4R12X4_422_UNORM_4PACK16      = 1000156020,
		B12X4G12X4R12X4G12X4_422_UNORM_4PACK16      = 1000156021,
		G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16  = 1000156022,
		G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16   = 1000156023,
		G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16  = 1000156024,
		G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16   = 1000156025,
		G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16  = 1000156026,
		G16B16G16R16_422_UNORM                      = 1000156027,
		B16G16R16G16_422_UNORM                      = 1000156028,
		G16_B16_R16_3PLANE_420_UNORM                = 1000156029,
		G16_B16R16_2PLANE_420_UNORM                 = 1000156030,
		G16_B16_R16_3PLANE_422_UNORM                = 1000156031,
		G16_B16R16_2PLANE_422_UNORM                 = 1000156032,
		G16_B16_R16_3PLANE_444_UNORM                = 1000156033,
		PVRTC1_2BPP_UNORM_BLOCK_IMG                 = 1000054000,
		PVRTC1_4BPP_UNORM_BLOCK_IMG                 = 1000054001,
		PVRTC2_2BPP_UNORM_BLOCK_IMG                 = 1000054002,
		PVRTC2_4BPP_UNORM_BLOCK_IMG                 = 1000054003,
		PVRTC1_2BPP_SRGB_BLOCK_IMG                  = 1000054004,
		PVRTC1_4BPP_SRGB_BLOCK_IMG                  = 1000054005,
		PVRTC2_2BPP_SRGB_BLOCK_IMG                  = 1000054006,
		PVRTC2_4BPP_SRGB_BLOCK_IMG                  = 1000054007,
		ASTC_4x4_SFLOAT_BLOCK_EXT                   = 1000066000,
		ASTC_5x4_SFLOAT_BLOCK_EXT                   = 1000066001,
		ASTC_5x5_SFLOAT_BLOCK_EXT                   = 1000066002,
		ASTC_6x5_SFLOAT_BLOCK_EXT                   = 1000066003,
		ASTC_6x6_SFLOAT_BLOCK_EXT                   = 1000066004,
		ASTC_8x5_SFLOAT_BLOCK_EXT                   = 1000066005,
		ASTC_8x6_SFLOAT_BLOCK_EXT                   = 1000066006,
		ASTC_8x8_SFLOAT_BLOCK_EXT                   = 1000066007,
		ASTC_10x5_SFLOAT_BLOCK_EXT                  = 1000066008,
		ASTC_10x6_SFLOAT_BLOCK_EXT                  = 1000066009,
		ASTC_10x8_SFLOAT_BLOCK_EXT                  = 1000066010,
		ASTC_10x10_SFLOAT_BLOCK_EXT                 = 1000066011,
		ASTC_12x10_SFLOAT_BLOCK_EXT                 = 1000066012,
		ASTC_12x12_SFLOAT_BLOCK_EXT                 = 1000066013,
		G8B8G8R8_422_UNORM_KHR                      = G8B8G8R8_422_UNORM,
		B8G8R8G8_422_UNORM_KHR                      = B8G8R8G8_422_UNORM,
		G8_B8_R8_3PLANE_420_UNORM_KHR               = G8_B8_R8_3PLANE_420_UNORM,
		G8_B8R8_2PLANE_420_UNORM_KHR                = G8_B8R8_2PLANE_420_UNORM,
		G8_B8_R8_3PLANE_422_UNORM_KHR               = G8_B8_R8_3PLANE_422_UNORM,
		G8_B8R8_2PLANE_422_UNORM_KHR                = G8_B8R8_2PLANE_422_UNORM,
		G8_B8_R8_3PLANE_444_UNORM_KHR               = G8_B8_R8_3PLANE_444_UNORM,
		R10X6_UNORM_PACK16_KHR                      = R10X6_UNORM_PACK16,
		R10X6G10X6_UNORM_2PACK16_KHR                = R10X6G10X6_UNORM_2PACK16,
		R10X6G10X6B10X6A10X6_UNORM_4PACK16_KHR      = R10X6G10X6B10X6A10X6_UNORM_4PACK16,
		G10X6B10X6G10X6R10X6_422_UNORM_4PACK16_KHR  = G10X6B10X6G10X6R10X6_422_UNORM_4PACK16,
		B10X6G10X6R10X6G10X6_422_UNORM_4PACK16_KHR  = B10X6G10X6R10X6G10X6_422_UNORM_4PACK16,
		G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_420_UNORM_3PACK16,
		G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_420_UNORM_3PACK16,
		G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_422_UNORM_3PACK16,
		G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16_KHR = G10X6_B10X6R10X6_2PLANE_422_UNORM_3PACK16,
		G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16_KHR = G10X6_B10X6_R10X6_3PLANE_444_UNORM_3PACK16,
		R12X4_UNORM_PACK16_KHR                      = R12X4_UNORM_PACK16,
		R12X4G12X4_UNORM_2PACK16_KHR                = R12X4G12X4_UNORM_2PACK16,
		R12X4G12X4B12X4A12X4_UNORM_4PACK16_KHR      = R12X4G12X4B12X4A12X4_UNORM_4PACK16,
		G12X4B12X4G12X4R12X4_422_UNORM_4PACK16_KHR  = G12X4B12X4G12X4R12X4_422_UNORM_4PACK16,
		B12X4G12X4R12X4G12X4_422_UNORM_4PACK16_KHR  = B12X4G12X4R12X4G12X4_422_UNORM_4PACK16,
		G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_420_UNORM_3PACK16,
		G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_420_UNORM_3PACK16,
		G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_422_UNORM_3PACK16,
		G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16_KHR = G12X4_B12X4R12X4_2PLANE_422_UNORM_3PACK16,
		G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16_KHR = G12X4_B12X4_R12X4_3PLANE_444_UNORM_3PACK16,
		G16B16G16R16_422_UNORM_KHR                  = G16B16G16R16_422_UNORM,
		B16G16R16G16_422_UNORM_KHR                  = B16G16R16G16_422_UNORM,
		G16_B16_R16_3PLANE_420_UNORM_KHR            = G16_B16_R16_3PLANE_420_UNORM,
		G16_B16R16_2PLANE_420_UNORM_KHR             = G16_B16R16_2PLANE_420_UNORM,
		G16_B16_R16_3PLANE_422_UNORM_KHR            = G16_B16_R16_3PLANE_422_UNORM,
		G16_B16R16_2PLANE_422_UNORM_KHR             = G16_B16R16_2PLANE_422_UNORM,
		G16_B16_R16_3PLANE_444_UNORM_KHR            = G16_B16_R16_3PLANE_444_UNORM,
		BEGIN_RANGE                                 = UNDEFINED,
		END_RANGE                                   = ASTC_12x12_SRGB_BLOCK,
		RANGE_SIZE                                  = (ASTC_12x12_SRGB_BLOCK - UNDEFINED + 1),
		MAX_ENUM = 0x7FFFFFFF
	};

	enum class ImageViewType
	{
		DIM_1           = 0,
		DIM_2           = 1,
		DIM_3           = 2,
		CUBE            = 3,
		DIM_1_ARRAY     = 4,
		DIM_2_ARRAY     = 5,
		CUBE_ARRAY      = 6,
		BEGIN_RANGE     = DIM_1,
		END_RANGE       = CUBE_ARRAY,
		RANGE_SIZE      = (CUBE_ARRAY - DIM_1 + 1),
		MAX_ENUM        = 0x7FFFFFFF
	};

	enum class ImageAspect
	{
		COLOR_BIT               = 0x00000001,
		DEPTH_BIT               = 0x00000002,
		STENCIL_BIT             = 0x00000004,
		METADATA_BIT            = 0x00000008,
		PLANE_0_BIT             = 0x00000010,
		PLANE_1_BIT             = 0x00000020,
		PLANE_2_BIT             = 0x00000040,
		MEMORY_PLANE_0_BIT_EXT  = 0x00000080,
		MEMORY_PLANE_1_BIT_EXT  = 0x00000100,
		MEMORY_PLANE_2_BIT_EXT  = 0x00000200,
		MEMORY_PLANE_3_BIT_EXT  = 0x00000400,
		PLANE_0_BIT_KHR         = PLANE_0_BIT,
		PLANE_1_BIT_KHR         = PLANE_1_BIT,
		PLANE_2_BIT_KHR         = PLANE_2_BIT,
		MAX_ENUM                = 0x7FFFFFFF
	};
	ENABLE_BITMASK_OPERATORS(ImageAspect);
}