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
	inline VkShaderStageFlags ConvertShaderStageVK(FShaderStage stage)
	{
		VkShaderStageFlags mask = 0;
		FLAG_CHECK(stage & FShaderStage::VERTEX,					mask |= VK_SHADER_STAGE_VERTEX_BIT);
		FLAG_CHECK(stage & FShaderStage::FRAGMENT,					mask |= VK_SHADER_STAGE_FRAGMENT_BIT);
		FLAG_CHECK(stage & FShaderStage::COMPUTE,					mask |= VK_SHADER_STAGE_COMPUTE_BIT);
		FLAG_CHECK(stage & FShaderStage::TESSELLATION_CONTROL,		mask |= VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT);
		FLAG_CHECK(stage & FShaderStage::TESSELLATION_EVALUATION,	mask |= VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT);
		FLAG_CHECK(stage & FShaderStage::GEOMETRY,					mask |= VK_SHADER_STAGE_GEOMETRY_BIT);
		FLAG_CHECK(stage & FShaderStage::ANY_HIT,					mask |= VK_SHADER_STAGE_RAYGEN_BIT_KHR);
		FLAG_CHECK(stage & FShaderStage::CLOSEST_HIT,				mask |= VK_SHADER_STAGE_ANY_HIT_BIT_KHR);
		FLAG_CHECK(stage & FShaderStage::MISS,						mask |= VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR);
		FLAG_CHECK(stage & FShaderStage::INTERSECTION,				mask |= VK_SHADER_STAGE_MISS_BIT_KHR);
		FLAG_CHECK(stage & FShaderStage::INTERSECTION,				mask |= VK_SHADER_STAGE_INTERSECTION_BIT_KHR);
		FLAG_CHECK(stage & FShaderStage::CALLABLE,					mask |= VK_SHADER_STAGE_CALLABLE_BIT_KHR);
		FLAG_CHECK(stage & FShaderStage::TASK,						mask |= VK_SHADER_STAGE_TASK_BIT_NV);
		FLAG_CHECK(stage & FShaderStage::MESH,						mask |= VK_SHADER_STAGE_MESH_BIT_NV);
		return mask;
	}

	inline VkShaderStageFlagBits ConvertShaderStageBitsVK(FShaderStage stage)
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

	inline VkBufferUsageFlags ConvertBufferUsageVK(FBufferUsage bufferUsage)
	{
		VkBufferUsageFlags mask = 0;
		FLAG_CHECK(bufferUsage & FBufferUsage::VERTEX_BUFFER,	mask |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
		FLAG_CHECK(bufferUsage & FBufferUsage::INDEX_BUFFER,	mask |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT);
		FLAG_CHECK(bufferUsage & FBufferUsage::STORAGE_BUFFER,	mask |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT);
		FLAG_CHECK(bufferUsage & FBufferUsage::UNIFORM_BUFFER,	mask |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT);
		FLAG_CHECK(bufferUsage & FBufferUsage::TRANSFER_SRC,	mask |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		FLAG_CHECK(bufferUsage & FBufferUsage::TRANSFER_DST,	mask |= VK_BUFFER_USAGE_TRANSFER_DST_BIT);
		FLAG_CHECK(bufferUsage & FBufferUsage::RAY_TRACING,		mask |= VK_BUFFER_USAGE_RAY_TRACING_BIT_NV);
		FLAG_CHECK(bufferUsage & FBufferUsage::INDIRECT_BUFFER,	mask |= VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);
		return mask;
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
		case EFormat::B8G8R8A8_UNORM:		return VK_FORMAT_B8G8R8A8_UNORM;
		case EFormat::D24_UNORM_S8_UINT:	return VK_FORMAT_D24_UNORM_S8_UINT;
		default:							return VK_FORMAT_UNDEFINED;
		}
	}

	inline VkImageUsageFlags ConvertTextureUsageVK(FTextureUsage textureUsage)
	{
		VkImageUsageFlags mask = 0;
		FLAG_CHECK(textureUsage & FTextureUsage::TRANSFER_SRC,				mask |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT);
		FLAG_CHECK(textureUsage & FTextureUsage::TRANSFER_DST,				mask |= VK_IMAGE_USAGE_TRANSFER_DST_BIT);
		FLAG_CHECK(textureUsage & FTextureUsage::SAMPLED,					mask |= VK_IMAGE_USAGE_SAMPLED_BIT);
		FLAG_CHECK(textureUsage & FTextureUsage::STORAGE,					mask |= VK_IMAGE_USAGE_STORAGE_BIT);
		FLAG_CHECK(textureUsage & FTextureUsage::COLOR_ATTACHMENT,			mask |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT);
		FLAG_CHECK(textureUsage & FTextureUsage::INPUT_ATTACHMENT,			mask |= VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT);
		FLAG_CHECK(textureUsage & FTextureUsage::DEPTH_STENCIL_ATTACHMENT,	mask |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT);
		FLAG_CHECK(textureUsage & FTextureUsage::SHADING_RATE,				mask |= VK_IMAGE_USAGE_SHADING_RATE_IMAGE_BIT_NV);
		FLAG_CHECK(textureUsage & FTextureUsage::FRAGMENT_DENSITY,			mask |= VK_IMAGE_USAGE_FRAGMENT_DENSITY_MAP_BIT_EXT);
		return mask;
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
		FLAG_CHECK(imageViewFlag & FImageViewFlag::DEPTH_STENCIL,		mask |= VK_IMAGE_ASPECT_DEPTH_BIT);
		FLAG_CHECK(imageViewFlag & FImageViewFlag::COLOR,				mask |= VK_IMAGE_ASPECT_COLOR_BIT);
		return mask;
	}

	inline VkPipelineStageFlags ConvertPipelineStageFlagsVK(FPipelineStage pipelineStage)
	{
		VkPipelineStageFlags mask = 0;
		FLAG_CHECK(pipelineStage & FPipelineStage::TOP_OF_PIPE,				mask |= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::DRAW_INDIRECT,			mask |= VK_PIPELINE_STAGE_DRAW_INDIRECT_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::VERTEX_INPUT,			mask |= VK_PIPELINE_STAGE_VERTEX_INPUT_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::VERTEX_SHADER,			mask |= VK_PIPELINE_STAGE_VERTEX_SHADER_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::FRAGMENT_SHADER,			mask |= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::EARLY_FRAGMENT_TEST,		mask |= VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::LATE_FRAGMENT_TEST,		mask |= VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::COLOR_ATTACHMENT_OUTPUT,	mask |= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::COMPUTE_SHADER,			mask |= VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::TRANSFER,				mask |= VK_PIPELINE_STAGE_TRANSFER_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::BOTTOM_OF_PIPE,			mask |= VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::ALL_GRAPHICS,			mask |= VK_PIPELINE_STAGE_ALL_GRAPHICS_BIT);
		FLAG_CHECK(pipelineStage & FPipelineStage::ALL_COMMANDS,			mask |= VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
		return mask;
	}

	inline VkCommandBufferLevel ConvertCommandBufferType(ECommandBufferLevel commandBufferType)
	{
		switch (commandBufferType)
		{
			case ECommandBufferLevel::PRIMARY:		return VK_COMMAND_BUFFER_LEVEL_PRIMARY;
			case ECommandBufferLevel::SECONDARY:	return VK_COMMAND_BUFFER_LEVEL_SECONDARY;
			default:								return VK_COMMAND_BUFFER_LEVEL_MAX_ENUM;
		}
	}

	inline VkCommandBufferUsageFlags ConvertCommandBufferUsage(FCommandBufferFlag commandBufferFlag)
	{
		VkCommandBufferUsageFlags mask = 0;
		FLAG_CHECK(commandBufferFlag & FCommandBufferFlag::ONE_TIME_SUBMIT, mask |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		return mask;
	}

	inline VkFenceCreateFlags ConvertFenceFlagsVK(FFenceFlag fenceFlag)
	{
		VkFenceCreateFlags mask = 0;
		FLAG_CHECK(fenceFlag & FFenceFlag::SIGNALED, mask |= VK_FENCE_CREATE_SIGNALED_BIT);
		return mask;
	}

	inline VkFilter ConvertFilterVK(EFilter filter)
	{
		switch (filter)
		{
			case EFilter::NEAREST:	return VK_FILTER_NEAREST;
			case EFilter::LINEAR:	return VK_FILTER_LINEAR;
			case EFilter::CUBIC:	return VK_FILTER_CUBIC_IMG;
			default:				return VK_FILTER_NEAREST;
		}
	}

	inline VkSamplerAddressMode ConvertSamplerAddressModeVK(ESamplerAddressMode addressMode)
	{
		switch (addressMode)
		{
			case ESamplerAddressMode::REPEAT:				return VK_SAMPLER_ADDRESS_MODE_REPEAT;
			case ESamplerAddressMode::MIRROR_REPEAT:		return VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
			case ESamplerAddressMode::CLAMP_TO_EDGE:		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
			case ESamplerAddressMode::CLAMP_TO_BORDER:		return VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
			case ESamplerAddressMode::MIRROR_CLAMP_TO_EDGE:	return VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
			default:										return VK_SAMPLER_ADDRESS_MODE_REPEAT;
		}
	}

	inline VkSamplerMipmapMode ConvertSamplerMipmapModeVK(ESamplerMipmapMode mipmapMode)
	{
		switch (mipmapMode)
		{
			case ESamplerMipmapMode::NEAREST:	return VK_SAMPLER_MIPMAP_MODE_NEAREST;
			case ESamplerMipmapMode::LINEAR:	return VK_SAMPLER_MIPMAP_MODE_LINEAR;
			default:							return VK_SAMPLER_MIPMAP_MODE_NEAREST;
		}
	}

	inline VkBorderColor ConvertBorderColorVK(EBorderColor borderColor)
	{
		switch (borderColor)
		{
			case EBorderColor::FLOAT_TRANSPARANT_BLACK:	return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
			case EBorderColor::INT_TRANSPARENT_BLACK:	return VK_BORDER_COLOR_INT_TRANSPARENT_BLACK;
			case EBorderColor::FLOAT_OPAQUE_BLACK:		return VK_BORDER_COLOR_FLOAT_OPAQUE_BLACK;
			case EBorderColor::INT_OPAQUE_BLACK:		return VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			case EBorderColor::FLOAT_OPAQUE_WHITE:		return VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
			case EBorderColor::INT_OPAQUE_WHITE:		return VK_BORDER_COLOR_INT_OPAQUE_WHITE;
			default:									return VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK;
		}
	}

	inline VkDescriptorType ConvertDescriptorTypeVK(EDescriptorType descriptorType)
	{
		switch (descriptorType)
		{
			case EDescriptorType::SAMPLER:					return VK_DESCRIPTOR_TYPE_SAMPLER;
			case EDescriptorType::COMBINED_IMAGE_SAMPLER:	return VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			case EDescriptorType::SAMPLED_IMAGE:			return VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
			case EDescriptorType::STORAGE_IMAGE:			return VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			case EDescriptorType::UNIFORM_TEXEL_BUFFER:		return VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER;
			case EDescriptorType::STORAGE_TEXEL_BUFFER:		return VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER;
			case EDescriptorType::UNIFORM_BUFFER:			return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			case EDescriptorType::STORAGE_BUFFER:			return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			case EDescriptorType::UNIFORM_BUFFER_DYNAMIC:	return VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
			case EDescriptorType::STORAGE_BUFFER_DYNAMIC:	return VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC;
			case EDescriptorType::INPUT_ATTACHMENT:			return VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT;
			default:										return VK_DESCRIPTOR_TYPE_MAX_ENUM;
		}
	}

	inline VkAttachmentLoadOp ConvertLoadOpVK(ELoadOp loadOp)
	{
		switch (loadOp)
		{
			case ELoadOp::LOAD:			return VK_ATTACHMENT_LOAD_OP_LOAD;
			case ELoadOp::DONT_CARE:	return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			case ELoadOp::CLEAR:		return VK_ATTACHMENT_LOAD_OP_CLEAR;
			default:					return VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		}
	}

	inline VkAttachmentStoreOp ConvertStoreOpVK(EStoreOp storeOp)
	{
		switch (storeOp)
		{
			case EStoreOp::STORE:		return VK_ATTACHMENT_STORE_OP_STORE;
			case EStoreOp::DONT_CARE:	return VK_ATTACHMENT_STORE_OP_DONT_CARE;
			default:					return VK_ATTACHMENT_STORE_OP_DONT_CARE;
		}
	}

	inline VkImageLayout ConvertTextureLayoutVK(ETextureLayout textureLayout)
	{
		switch (textureLayout)
		{
		case ETextureLayout::UNDEFINED:										return VK_IMAGE_LAYOUT_UNDEFINED;
		case ETextureLayout::GENERAL:										return VK_IMAGE_LAYOUT_GENERAL;
		case ETextureLayout::COLOR_ATTACHMENT_OPTIMAL:						return VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		case ETextureLayout::DEPTH_STENCIL_ATTACHMENT_OPTIMAL:				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
		case ETextureLayout::DEPTH_STENCIL_READ_ONLY_OPTIMAL:				return VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
		case ETextureLayout::SHADER_READ_ONLY_OPTIMAL:						return VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		case ETextureLayout::TRANSFER_SRC_OPTIMAL:							return VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
		case ETextureLayout::TRANSFER_DST_OPTIMAL:							return VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		case ETextureLayout::PREINITIALIZED:								return VK_IMAGE_LAYOUT_PREINITIALIZED;
		case ETextureLayout::DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL:	return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL;
		case ETextureLayout::DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL:	return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL;
		case ETextureLayout::DEPTH_ATTACHMENT_OPTIMAL:						return VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
		case ETextureLayout::DEPTH_READ_ONLY_OPTIMAL:						return VK_IMAGE_LAYOUT_DEPTH_READ_ONLY_OPTIMAL;
		case ETextureLayout::STENCIL_ATTACHMENT_OPTIMAL:					return VK_IMAGE_LAYOUT_STENCIL_ATTACHMENT_OPTIMAL;
		case ETextureLayout::STENCIL_READ_ONLY_OPTIMAL:						return VK_IMAGE_LAYOUT_STENCIL_READ_ONLY_OPTIMAL;
		case ETextureLayout::PRESENT:										return VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
		case ETextureLayout::SHARED_PRESENT:								return VK_IMAGE_LAYOUT_SHARED_PRESENT_KHR;
		case ETextureLayout::SHADING_RATE_OPTIMAL:							return VK_IMAGE_LAYOUT_SHADING_RATE_OPTIMAL_NV;
		case ETextureLayout::FRAGMENT_DENSITY_MAP_OPTIMAL:					return VK_IMAGE_LAYOUT_FRAGMENT_DENSITY_MAP_OPTIMAL_EXT;
		default:															return VK_IMAGE_LAYOUT_UNDEFINED;
		}
	}

	inline VkAccessFlags ConvertAccessFlagVK(FAccessFlag accessFlag)
	{
		VkAccessFlags mask = 0;
		FLAG_CHECK(accessFlag & FAccessFlag::INDIRECT_COMMAND_READ, mask |= VK_ACCESS_INDIRECT_COMMAND_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::INDEX_READ, mask |= VK_ACCESS_INDEX_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::VERTEX_ATTRIBUTE_READ, mask |= VK_ACCESS_VERTEX_ATTRIBUTE_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::UNIFORM_READ, mask |= VK_ACCESS_UNIFORM_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::INPUT_ATTACHMENT_READ, mask |= VK_ACCESS_INPUT_ATTACHMENT_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::SHADER_READ, mask |= VK_ACCESS_SHADER_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::SHADER_WRITE, mask |= VK_ACCESS_SHADER_WRITE_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::COLOR_ATTACHMENT_READ, mask |= VK_ACCESS_COLOR_ATTACHMENT_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::COLOR_ATTACHMENT_WRITE, mask |= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::DEPTH_STENCIL_ATTACHMENT_READ, mask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::DEPTH_STENCIL_ATTACHMENT_WRITE, mask |= VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::TRANSFER_READ, mask |= VK_ACCESS_TRANSFER_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::TRANSFER_WRITE, mask |= VK_ACCESS_TRANSFER_WRITE_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::HOST_READ, mask |= VK_ACCESS_HOST_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::HOST_WRITE, mask |= VK_ACCESS_HOST_WRITE_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::MEMORY_READ, mask |= VK_ACCESS_MEMORY_READ_BIT);
		FLAG_CHECK(accessFlag & FAccessFlag::MEMORY_WRITE, mask |= VK_ACCESS_MEMORY_WRITE_BIT);
		return mask;
	}

	inline VkSampleCountFlagBits ConvertSampleCountVK(uint32 samples)
	{
		switch (samples)
		{
		case 1:		return VK_SAMPLE_COUNT_1_BIT;
		case 2:		return VK_SAMPLE_COUNT_2_BIT;
		case 4:		return VK_SAMPLE_COUNT_4_BIT;
		case 8:		return VK_SAMPLE_COUNT_8_BIT;
		case 16:	return VK_SAMPLE_COUNT_16_BIT;
		case 32:	return VK_SAMPLE_COUNT_32_BIT;
		case 64:	return VK_SAMPLE_COUNT_64_BIT;
		default:	return VK_SAMPLE_COUNT_1_BIT;
		}
	}

	inline VkPrimitiveTopology ConvertTopologyVK(ETopology topology)
	{
		switch (topology)
		{
		case ETopology::POINT_LIST:						return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case ETopology::LINE_LIST:						return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case ETopology::LINE_STRIP:						return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case ETopology::TRIANGLE_LIST:					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case ETopology::TRIANGLE_STRIP:					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case ETopology::TRIANGLE_FAN:					return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		case ETopology::LINE_LIST_WITH_ADJACENCY:		return VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		case ETopology::LINE_STRIP_WITH_ADJACENCY:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		case ETopology::TRIANGLE_LIST_WITH_ADJACENCY:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		case ETopology::TRIANGLE_STRIP_WITH_ADJACENCY:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		case ETopology::PATCH_LIST:						return VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		default:										return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		}
	}

	inline VkPolygonMode ConvertPolygonModeVK(EPolygonMode polygonMode)
	{
		switch (polygonMode)
		{
			case EPolygonMode::FILL:	return VK_POLYGON_MODE_FILL;
			case EPolygonMode::LINE:	return VK_POLYGON_MODE_LINE;
			case EPolygonMode::POINT:	return VK_POLYGON_MODE_POINT;
			default:					return VK_POLYGON_MODE_FILL;
		}
	}

	inline VkCullModeFlagBits ConvertCullModeVK(ECullMode cullMode)
	{
		switch (cullMode)
		{
			case ECullMode::FRONT:			return VK_CULL_MODE_FRONT_BIT;
			case ECullMode::BACK:			return VK_CULL_MODE_BACK_BIT;
			case ECullMode::FRONT_AND_BACK:	return VK_CULL_MODE_FRONT_AND_BACK;
			default:						return VK_CULL_MODE_NONE;
		}
	}

	inline VkCompareOp ConvertCompareOpVK(ECompareOp compareOp)
	{
		switch (compareOp)
		{
			case ECompareOp::NEVER:				return VK_COMPARE_OP_NEVER;
			case ECompareOp::LESS:				return VK_COMPARE_OP_LESS;
			case ECompareOp::EQUAL:				return VK_COMPARE_OP_EQUAL;
			case ECompareOp::LESS_OR_EQUAL:		return VK_COMPARE_OP_LESS_OR_EQUAL;
			case ECompareOp::GREATER:			return VK_COMPARE_OP_GREATER;
			case ECompareOp::NOT_EQUAL:			return VK_COMPARE_OP_NOT_EQUAL;
			case ECompareOp::GREATER_OR_EQUAL:	return VK_COMPARE_OP_GREATER_OR_EQUAL;
			case ECompareOp::ALWAYS:			return VK_COMPARE_OP_ALWAYS;
			default:							return VK_COMPARE_OP_NEVER;
		}
	}

	inline VkStencilOp ConvertStencilOpVK(EStencilOp stencilOp)
	{
		switch (stencilOp)
		{
			case EStencilOp::KEEP:					return VK_STENCIL_OP_KEEP;
			case EStencilOp::ZERO:					return VK_STENCIL_OP_ZERO;
			case EStencilOp::REPLACE:				return VK_STENCIL_OP_REPLACE;
			case EStencilOp::INCREMENT_AND_CLAMP:	return VK_STENCIL_OP_INCREMENT_AND_CLAMP;
			case EStencilOp::DECREMENT_AND_CLAMP:	return VK_STENCIL_OP_DECREMENT_AND_CLAMP;
			case EStencilOp::INVERT:				return VK_STENCIL_OP_INVERT;
			case EStencilOp::INCREMENT_AND_WRAP:	return VK_STENCIL_OP_INCREMENT_AND_WRAP;
			case EStencilOp::DECREMENT_AND_WRAP:	return VK_STENCIL_OP_DECREMENT_AND_WRAP;
			default:								return VK_STENCIL_OP_KEEP;
		}
	}

	inline VkBlendFactor ConvertBlendFactorVK(EBlendFactor blendFactor)
	{
		switch (blendFactor)
		{
			case EBlendFactor::ZERO:						return VK_BLEND_FACTOR_ZERO;
			case EBlendFactor::ONE:							return VK_BLEND_FACTOR_ONE;
			case EBlendFactor::SRC_COLOR:					return VK_BLEND_FACTOR_SRC_COLOR;
			case EBlendFactor::ONE_MINUS_SRC_COLOR:			return VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
			case EBlendFactor::DST_COLOR:					return VK_BLEND_FACTOR_DST_COLOR;
			case EBlendFactor::ONE_MINUS_DST_COLOR:			return VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
			case EBlendFactor::SRC_ALPHA:					return VK_BLEND_FACTOR_SRC_ALPHA;
			case EBlendFactor::ONE_MINUS_SRC_ALPHA:			return VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
			case EBlendFactor::DST_ALPHA:					return VK_BLEND_FACTOR_DST_ALPHA;
			case EBlendFactor::ONE_MINUS_DST_ALPHA:			return VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
			case EBlendFactor::CONSTANT_COLOR:				return VK_BLEND_FACTOR_CONSTANT_COLOR;
			case EBlendFactor::ONE_MINUS_CONSTANT_COLOR:	return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
			case EBlendFactor::CONSTANT_ALPHA:				return VK_BLEND_FACTOR_CONSTANT_ALPHA;
			case EBlendFactor::ONE_MINUS_CONSTANT_ALPHA:	return VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
			case EBlendFactor::SRC_ALPHA_SATURATE:			return VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
			case EBlendFactor::SRC1_COLOR:					return VK_BLEND_FACTOR_SRC1_COLOR;
			case EBlendFactor::ONE_MINUS_SRC1_COLOR:		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
			case EBlendFactor::SRC1_ALPHA:					return VK_BLEND_FACTOR_SRC1_ALPHA;
			case EBlendFactor::ONE_MINUS_SRC1_ALPHA:		return VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
			default:										return VK_BLEND_FACTOR_ZERO;
		}
	}

	inline VkBlendOp ConvertBlendOpVK(EBlendOp blendOp)
	{
		switch (blendOp)
		{
			case EBlendOp::ADD:					return VK_BLEND_OP_ADD;
			case EBlendOp::SUBTRACT:			return VK_BLEND_OP_SUBTRACT;
			case EBlendOp::REVERSE_SUBTRACT:	return VK_BLEND_OP_REVERSE_SUBTRACT;
			case EBlendOp::MIN:					return VK_BLEND_OP_MIN;
			case EBlendOp::MAX:					return VK_BLEND_OP_MAX;
			default:							return VK_BLEND_OP_ADD;
		}
	}

	inline VkColorComponentFlags ConvertColorComponentVK(FColorComponentFlag colorComponent)
	{
		VkColorComponentFlags mask = 0;
		FLAG_CHECK(colorComponent & FColorComponentFlag::RED, mask |= VK_COLOR_COMPONENT_R_BIT);
		FLAG_CHECK(colorComponent & FColorComponentFlag::GREEN, mask |= VK_COLOR_COMPONENT_G_BIT);
		FLAG_CHECK(colorComponent & FColorComponentFlag::BLUE, mask |= VK_COLOR_COMPONENT_B_BIT);
		FLAG_CHECK(colorComponent & FColorComponentFlag::ALPHA, mask |= VK_COLOR_COMPONENT_A_BIT);
		return mask;
	}

	inline VkLogicOp ConvertLogicOpVK(ELogicOp logicOp)
	{
		switch (logicOp)
		{
			case ELogicOp::CLEAR:			return VK_LOGIC_OP_CLEAR;
			case ELogicOp::AND:				return VK_LOGIC_OP_AND;
			case ELogicOp::AND_REVERSE:		return VK_LOGIC_OP_AND_REVERSE;
			case ELogicOp::COPY:			return VK_LOGIC_OP_COPY;
			case ELogicOp::AND_INVERTED:	return VK_LOGIC_OP_AND_INVERTED;
			case ELogicOp::NO_OP:			return VK_LOGIC_OP_NO_OP;
			case ELogicOp::XOR:				return VK_LOGIC_OP_XOR;
			case ELogicOp::OR:				return VK_LOGIC_OP_OR;
			case ELogicOp::NOR:				return VK_LOGIC_OP_NOR;
			case ELogicOp::EQUIVALENT:		return VK_LOGIC_OP_EQUIVALENT;
			case ELogicOp::INVERT:			return VK_LOGIC_OP_INVERT;
			case ELogicOp::OR_REVERSE:		return VK_LOGIC_OP_OR_REVERSE;
			case ELogicOp::COPY_INVERTED:	return VK_LOGIC_OP_COPY_INVERTED;
			case ELogicOp::OR_INVERTED:		return VK_LOGIC_OP_OR_INVERTED;
			case ELogicOp::NAND:			return VK_LOGIC_OP_NAND;
			case ELogicOp::SET:				return VK_LOGIC_OP_SET;
			default:						return VK_LOGIC_OP_CLEAR;
		}
	}

	inline VkVertexInputRate ConvertVertexInputRateVK(EVertexInputRate inputRate)
	{
		switch (inputRate)
		{
			case EVertexInputRate::VERTEX:		return VK_VERTEX_INPUT_RATE_VERTEX;
			case EVertexInputRate::INSTANCE:	return VK_VERTEX_INPUT_RATE_INSTANCE;
			default:							return VK_VERTEX_INPUT_RATE_VERTEX;
		}
	}

	inline VkPipelineBindPoint ConvertPipelineTypeVK(EPipelineType pipelineType)
	{
		switch (pipelineType)
		{
			case EPipelineType::GRAPHICS:		return VK_PIPELINE_BIND_POINT_GRAPHICS;
			case EPipelineType::COMPUTE:		return VK_PIPELINE_BIND_POINT_COMPUTE;
			case EPipelineType::RAY_TRACING:	return VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR;
			default:							return VK_PIPELINE_BIND_POINT_GRAPHICS;
		}
	}

	enum class BufferType
	{
		SAMPLER					= 0,
		COMBINED_IMAGE_SAMPLER	= 1,
		SAMPLED_IMAGE			= 2,
		STORAGE_IMAGE			= 3,
		UNIFORM_TEXEL			= 4,
		STORAGE_TEXEL			= 5,
		UNIFORM					= 6,
		STORAGE					= 7,
		UNIFORM_DYNAMIC			= 8,
		STORAGE_DYNAMIC			= 9,
		INPUT_ATTACHMENT		= 10,
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
}