#pragma once

#include <vulkan/vulkan.h>
#include "Platform/Vulkan/VmaInclude.h"
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
	inline VkShaderStageFlagBits ConvertShaderStageVK(FShaderStage stage)
	{
		switch (stage)
		{
		case FShaderStage::VERTEX:					return VK_SHADER_STAGE_VERTEX_BIT;
		case FShaderStage::FRAGMENT:				return VK_SHADER_STAGE_FRAGMENT_BIT;
		case FShaderStage::COMPUTE:					return VK_SHADER_STAGE_COMPUTE_BIT;
		case FShaderStage::TESSELLATION_CONTROL:	return VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		case FShaderStage::TESSELLATION_EVALUATION:	return VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		case FShaderStage::GEOMETRY:				return VK_SHADER_STAGE_GEOMETRY_BIT;
		case FShaderStage::RAYGEN:					return VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		case FShaderStage::ANY_HIT:					return VK_SHADER_STAGE_ANY_HIT_BIT_KHR;
		case FShaderStage::CLOSEST_HIT:				return VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		case FShaderStage::MISS:					return VK_SHADER_STAGE_MISS_BIT_KHR;
		case FShaderStage::INTERSECTION:			return VK_SHADER_STAGE_INTERSECTION_BIT_KHR;
		case FShaderStage::CALLABLE:				return VK_SHADER_STAGE_CALLABLE_BIT_KHR;
		case FShaderStage::TASK:					return VK_SHADER_STAGE_TASK_BIT_NV;
		case FShaderStage::MESH:					return VK_SHADER_STAGE_MESH_BIT_NV;
		default:									return VK_SHADER_STAGE_ALL;
		}
	}

	inline VkBufferUsageFlagBits ConvertBufferUsageVK(FBufferUsage bufferUsage)
	{
		switch (bufferUsage)
		{
		case FBufferUsage::VERTEX_BUFFER:		return VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		case FBufferUsage::INDEX_BUFFER:		return VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		case FBufferUsage::STORAGE_BUFFER:		return VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		case FBufferUsage::UNIFORM_BUFFER:		return VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		case FBufferUsage::TRANSFER_SRC:		return VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		case FBufferUsage::TRANSFER_DST:		return VK_BUFFER_USAGE_TRANSFER_DST_BIT;
		case FBufferUsage::RAY_TRACING:			return VK_BUFFER_USAGE_RAY_TRACING_BIT_NV;
		case FBufferUsage::INDIRECT_BUFFER:		return VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT;
		default:								return VK_BUFFER_USAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	inline VmaMemoryUsage ConvertMemoryUsageVMA(EMemoryUsage memUsage)
	{
		switch (memUsage)
		{
		case EMemoryUsage::CPU_VISIBLE:			return VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_ONLY;
		case EMemoryUsage::GPU_ONLY:			return VmaMemoryUsage::VMA_MEMORY_USAGE_GPU_ONLY;
		case EMemoryUsage::CPU_GPU_MAPPABLE:	return VmaMemoryUsage::VMA_MEMORY_USAGE_CPU_TO_GPU;
		default:								return VMA_MEMORY_USAGE_UNKNOWN;
		}
	}

	inline VkFormat ConvertFormatVK(EFormat format)
	{
		switch (format)
		{
		case EFormat::R8G8B8A8_UNORM:		return VK_FORMAT_R8G8B8A8_UNORM;
		case EFormat::D24_UNORM_S8_UINT:	return VK_FORMAT_D24_UNORM_S8_UINT;
		default:							return VK_FORMAT_UNDEFINED;
		}
	}

	inline VkImageUsageFlagBits ConvertTextureUsageVK(FTextureUsage textureUsage)
	{
		switch (textureUsage)
		{
		case FTextureUsage::TRANSFER_SRC:				return VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
		case FTextureUsage::TRANSFER_DST:				return VK_IMAGE_USAGE_TRANSFER_DST_BIT;
		case FTextureUsage::SAMPLED:					return VK_IMAGE_USAGE_SAMPLED_BIT;
		case FTextureUsage::STORAGE:					return VK_IMAGE_USAGE_STORAGE_BIT;
		case FTextureUsage::COLOR_ATTACHMENT:			return VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		case FTextureUsage::INPUT_ATTACHMENT:			return VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;
		case FTextureUsage::DEPTH_STENCIL_ATTACHMENT:	return VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		case FTextureUsage::SHADING_RATE:				return VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV;
		case FTextureUsage::FRAGMENT_DENSITY:			return VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT;
		default:										return VK_IMAGE_USAGE_FLAG_BITS_MAX_ENUM;
		}
	}

	inline VkImageType ConvertTextureDimVK(ETextureDim textureDim)
	{
		switch (textureDim)
		{
			case ETextureDim::DIM_1D:		return VK_IMAGE_TYPE_1D;
			case ETextureDim::DIM_2D:		return VK_IMAGE_TYPE_2D;
			case ETextureDim::DIM_3D:		return VK_IMAGE_TYPE_3D;
			default:						return VK_IMAGE_TYPE_MAX_ENUM;
		}
	}

	inline VkImageViewType ConvertImageViewTypeVK(EImageViewType imageViewType)
	{
		switch (imageViewType)
		{
			case EImageViewType::TYPE_1D:			return VK_IMAGE_VIEW_TYPE_1D;
			case EImageViewType::TYPE_2D:			return VK_IMAGE_VIEW_TYPE_2D;
			case EImageViewType::TYPE_3D:			return VK_IMAGE_VIEW_TYPE_3D;
			case EImageViewType::TYPE_CUBE:			return VK_IMAGE_VIEW_TYPE_CUBE;
			case EImageViewType::TYPE_CUBE_ARRAY:	return VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;
			case EImageViewType::TYPE_1D_ARRAY:		return VK_IMAGE_VIEW_TYPE_1D_ARRAY;
			case EImageViewType::TYPE_2D_ARRAY:		return VK_IMAGE_VIEW_TYPE_2D_ARRAY;
			default:								return VK_IMAGE_VIEW_TYPE_MAX_ENUM;
		}
	}

	inline VkImageAspectFlags ConvertImageViewFlagsVK(FImageViewFlag imageViewFlag)
	{
		VkImageAspectFlags mask = 0;
		FLAG_CHECK(imageViewFlag | FImageViewFlag::DEPTH_STENCIL, mask |= VK_IMAGE_ASPECT_DEPTH_BIT);
		FLAG_CHECK(imageViewFlag | FImageViewFlag::RENDER_TARGET, mask |= VK_IMAGE_ASPECT_COLOR_BIT);
		FLAG_CHECK(imageViewFlag | FImageViewFlag::SHADER_RESOURCE, mask |= VK_IMAGE_ASPECT_COLOR_BIT);
		FLAG_CHECK(imageViewFlag | FImageViewFlag::UNORDERED_ACCESS, mask |= VK_IMAGE_ASPECT_COLOR_BIT);
		return mask;
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