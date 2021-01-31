#pragma once

/*
	This file defines all of the types that is commonly used for all rendering platforms
*/

#include "Poly/Core/Core.h"
#include "Poly/Core/PolyUtils.h"

namespace Poly
{
	const constexpr uint32 EXTERNAL_SUBPASS = ~0U;

	enum class FShaderStage : uint32
	{
		NONE					= 0,
		VERTEX					= FLAG(1),
		FRAGMENT				= FLAG(2),
		COMPUTE					= FLAG(3),
		TESSELLATION_CONTROL	= FLAG(4),
		TESSELLATION_EVALUATION	= FLAG(5),
		GEOMETRY				= FLAG(6),
		RAYGEN					= FLAG(7),
		ANY_HIT					= FLAG(8),
		CLOSEST_HIT				= FLAG(9),
		MISS					= FLAG(10),
		INTERSECTION			= FLAG(11),
		CALLABLE				= FLAG(12),
		TASK					= FLAG(13),
		MESH					= FLAG(14)
	};
	ENABLE_BITMASK_OPERATORS(FShaderStage);

	enum class EMemoryUsage
	{
		UNKNOWN				= 0,
		GPU_ONLY			= 1,
		CPU_VISIBLE			= 2,
		CPU_GPU_MAPPABLE	= 3 // Prefer to only use this for debug
	};

	enum class FBufferUsage : uint32
	{
		NONE					= 0,
		VERTEX_BUFFER			= FLAG(1),
		INDEX_BUFFER			= FLAG(2),
		STORAGE_BUFFER			= FLAG(3),			// Same as UNORDERED_ACCESS_BUFFER (Directx name)
		UNORDERED_ACCESS_BUFFER	= STORAGE_BUFFER,	// Same as STORAGE_BUFFER (VK name)
		UNIFORM_BUFFER			= FLAG(4),			// Same as CONSTANT_BUFFER (Directx name)
		CONSTANT_BUFFER			= UNIFORM_BUFFER,	// Same as UNIFORM_BUFFER (VK name)
		TRANSFER_SRC			= FLAG(5),			// Same as COPY_SRC (Directx name)
		COPY_SRC				= TRANSFER_SRC,		// Same as TRANSFER_SRC (VK name)
		TRANSFER_DST			= FLAG(6),			// Same as COPY_DST (Directx name)
		COPY_DST				= TRANSFER_DST,		// Same as TRANSFER_DST (VK name)
		RAY_TRACING				= FLAG(7),
		INDIRECT_BUFFER			= FLAG(8)
	};
	ENABLE_BITMASK_OPERATORS(FBufferUsage);

	enum class EFormat
	{
		UNDEFINED			= 0,
		R8G8B8A8_UNORM		= 1,
		B8G8R8A8_UNORM		= 2,	// Common for swap chain surface
		D24_UNORM_S8_UINT	= 3		// Common for depth-stencil
	};

	enum class FTextureUsage : uint32
	{
		NONE						= 0,
		TRANSFER_SRC				= FLAG(1),		// Same as COPY_SRC (Directx name)
		COPY_SRC					= TRANSFER_SRC,	// Same as TRANSFER_SRC (VK name)
		TRANSFER_DST				= FLAG(2),		// Same as COPY_DST (Directx name)
		COPY_DST					= TRANSFER_DST,	// Same as TRANSFER_DST (VK name)
		SAMPLED						= FLAG(3),
		SHADER_RESOURCE				= SAMPLED,
		STORAGE						= FLAG(4),		// Same as UNORDERED_ACCESS (Directx name)
		UNORDERED_ACCESS			= STORAGE,		// Same as STORAGE (VK name)
		COLOR_ATTACHMENT			= FLAG(5),		// Same as RENDER_TARGET (Directx name)
		RENDER_TARGET				= COLOR_ATTACHMENT,	// Same as COLOR_ATTACHMENT (VK name)
		INPUT_ATTACHMENT			= FLAG(6),
		DEPTH_STENCIL_ATTACHMENT	= FLAG(7),
		SHADING_RATE				= FLAG(8),
		FRAGMENT_DENSITY			= FLAG(9)
	};
	ENABLE_BITMASK_OPERATORS(FTextureUsage);

	enum class ETextureDim
	{
		NONE	= 0,
		DIM_1D	= 1,
		DIM_2D	= 2,
		DIM_3D	= 3,
	};

	enum class EImageViewType
	{
		NONE			= 0,
		TYPE_1D			= 1,
		TYPE_2D 		= 2,
		TYPE_3D			= 3,
		TYPE_CUBE		= 4,
		TYPE_CUBE_ARRAY	= 5,
		TYPE_2D_ARRAY	= 6,
		TYPE_1D_ARRAY	= 7
	};

	enum class FImageViewFlag
	{
		NONE				= 0,
		SHADER_RESOURCE		= FLAG(1),
		RENDER_TARGET		= SHADER_RESOURCE,
		UNORDERED_ACCESS	= SHADER_RESOURCE,
		DEPTH_STENCIL		= FLAG(2)
	};
	ENABLE_BITMASK_OPERATORS(FImageViewFlag);

	// TODO: Consider if this should be class instead with sync and submit functions?
	enum class FQueueType
	{
		NONE			= 0,
		GRAPHICS		= FLAG(1),
		COMPUTE			= FLAG(2),
		TRANSFER		= FLAG(3),
		ALL				= 0xffff
	};
	ENABLE_BITMASK_OPERATORS(FQueueType);

	enum class FPipelineStage : uint32
	{
		NONE					= 0,
		TOP_OF_PIPE				= FLAG(1),
		DRAW_INDIRECT			= FLAG(2),
		VERTEX_INPUT			= FLAG(3),
		VERTEX_SHADER			= FLAG(4),
		FRAGMENT_SHADER			= FLAG(5),
		EARLY_FRAGMENT_TEST		= FLAG(6),
		LATE_FRAGMENT_TEST		= FLAG(7),
		COLOR_ATTACHMENT_OUTPUT	= FLAG(8),
		COMPUTE_SHADER			= FLAG(9),
		TRANSFER				= FLAG(10),
		BOTTOM_OF_PIPE			= FLAG(11),
		ALL_GRAPHICS			= FLAG(12),
		ALL_COMMANDS			= FLAG(13)
	};
	ENABLE_BITMASK_OPERATORS(FPipelineStage);

	enum class ECommandBufferLevel
	{
		NONE			= 0,
		PRIMARY			= 1,
		DIRECT			= PRIMARY,
		SECONDARY		= 2,
		BUNDLE			= SECONDARY
	};

	enum class FCommandBufferFlag
	{
		NONE			= 0,
		ONE_TIME_SUBMIT	= FLAG(1)
	};
	ENABLE_BITMASK_OPERATORS(FCommandBufferFlag);

	enum class FFenceFlag
	{
		NONE		= 0,
		SIGNALED	= FLAG(1)
	};
	ENABLE_BITMASK_OPERATORS(FFenceFlag);

	enum class EFilter
	{
		NONE	= 0,
		NEAREST	= 1,
		LINEAR	= 2,
		CUBIC	= 3
	};

	enum class ESamplerAddressMode
	{
		NONE					= 0,
		REPEAT					= 1,
		MIRROR_REPEAT			= 2,
		CLAMP_TO_EDGE			= 3,
		CLAMP_TO_BORDER			= 4,
		MIRROR_CLAMP_TO_EDGE	= 5
	};

	enum class ESamplerMipmapMode
	{
		NONE	= 0,
		NEAREST	= 1,
		LINEAR	= 2
	};

	enum class EBorderColor
	{
		NONE					= 0,
		FLOAT_TRANSPARANT_BLACK	= 1,
		INT_TRANSPARENT_BLACK	= 2,
		FLOAT_OPAQUE_BLACK		= 3,
		INT_OPAQUE_BLACK		= 4,
		FLOAT_OPAQUE_WHITE		= 5,
		INT_OPAQUE_WHITE		= 6
	};

	enum class EDescriptorType
	{
		NONE							= 0,
		SAMPLER							= 1,
		COMBINED_IMAGE_SAMPLER			= 2,
		SAMPLED_IMAGE					= 3,
		STORAGE_IMAGE					= 4,
		UNORDERED_ACCESS_IMAGE			= STORAGE_IMAGE,
		UNIFORM_TEXEL_BUFFER			= 5,
		CONSTANT_TEXEL_BUFFER			= UNIFORM_TEXEL_BUFFER,
		STORAGE_TEXEL_BUFFER			= 6,
		UNORDERED_ACCESS_TEXEL_BUFFER	= STORAGE_TEXEL_BUFFER,
		UNIFORM_BUFFER					= 7,
		CONSTANT_BUFFER					= UNIFORM_BUFFER,
		STORAGE_BUFFER					= 8,
		UNORDERED_ACCESS_BUFFER			= STORAGE_BUFFER,
		UNIFORM_BUFFER_DYNAMIC			= 9,
		CONSTANT_BUFFER_DYNAMIC			= UNIFORM_BUFFER_DYNAMIC,
		STORAGE_BUFFER_DYNAMIC			= 10,
		UNORDERED_ACCESS_BUFFER_DYNAMIC	= STORAGE_BUFFER_DYNAMIC,
		INPUT_ATTACHMENT				= 11,
		RENDER_TARGET					= INPUT_ATTACHMENT
	};

	enum class ELoadOp
	{
		NONE		= 0,
		LOAD		= 1,
		CLEAR		= 2,
		DONT_CARE	= 3
	};

	enum class EStoreOp
	{
		NONE		= 0,
		STORE		= 1,
		DONT_CARE	= 3,
	};

	enum class ETextureLayout
	{
		UNDEFINED									= 0,
		GENERAL										= 1,
		COLOR_ATTACHMENT_OPTIMAL					= 2,
		RENDER_TARGET_OPTIMAL						= COLOR_ATTACHMENT_OPTIMAL,
		DEPTH_STENCIL_ATTACHMENT_OPTIMAL			= 3,
		DEPTH_STENCIL_READ_ONLY_OPTIMAL				= 4,
		SHADER_READ_ONLY_OPTIMAL					= 5,
		TRANSFER_SRC_OPTIMAL						= 6,
		COPY_SRC_OPTIMAL							= TRANSFER_SRC_OPTIMAL,
		TRANSFER_DST_OPTIMAL						= 7,
		COPY_DST_OPTIMAL							= TRANSFER_DST_OPTIMAL,
		PREINITIALIZED								= 8,
		DEPTH_READ_ONLY_STENCIL_ATTACHMENT_OPTIMAL	= 9,
		DEPTH_ATTACHMENT_STENCIL_READ_ONLY_OPTIMAL	= 10,
		DEPTH_ATTACHMENT_OPTIMAL					= 11,
		DEPTH_READ_ONLY_OPTIMAL						= 12,
		STENCIL_ATTACHMENT_OPTIMAL					= 13,
		STENCIL_READ_ONLY_OPTIMAL					= 14,
		PRESENT										= 15,
		SHARED_PRESENT								= 16,
		SHADING_RATE_OPTIMAL						= 17,
		FRAGMENT_DENSITY_MAP_OPTIMAL				= 18
	};

	enum class FAccessFlag
	{
		NONE							= 0,
		INDIRECT_COMMAND_READ			= FLAG(1),
		INDEX_READ						= FLAG(2),
		VERTEX_ATTRIBUTE_READ			= FLAG(3),
		UNIFORM_READ					= FLAG(4),
		INPUT_ATTACHMENT_READ			= FLAG(5),
		SHADER_READ						= FLAG(6),
		SHADER_WRITE					= FLAG(7),
		COLOR_ATTACHMENT_READ			= FLAG(8),
		COLOR_ATTACHMENT_WRITE			= FLAG(9),
		DEPTH_STENCIL_ATTACHMENT_READ	= FLAG(10),
		DEPTH_STENCIL_ATTACHMENT_WRITE	= FLAG(11),
		TRANSFER_READ					= FLAG(12),
		TRANSFER_WRITE					= FLAG(13),
		HOST_READ						= FLAG(14),
		HOST_WRITE						= FLAG(15),
		MEMORY_READ						= FLAG(16),
		MEMORY_WRITE					= FLAG(17)
	};
	ENABLE_BITMASK_OPERATORS(FAccessFlag);

	enum class ETopology
	{
		NONE							= 0,
		POINT_LIST						= 1,
		LINE_LIST						= 2,
		LINE_STRIP						= 3,
		TRIANGLE_LIST					= 4,
		TRIANGLE_STRIP					= 5,
		TRIANGLE_FAN					= 6,
		LINE_LIST_WITH_ADJACENCY		= 7,
		LINE_STRIP_WITH_ADJACENCY		= 8,
		TRIANGLE_LIST_WITH_ADJACENCY	= 9,
		TRIANGLE_STRIP_WITH_ADJACENCY	= 10,
		PATCH_LIST 						= 11
	};

	enum class EPolygonMode
	{
		NONE	= 0,
		FILL	= 1,
		LINE	= 2,
		POINT	= 3
	};

	enum class ECullMode
	{
		NONE			= 0,
		FRONT			= 1,
		BACK			= 2,
		FRONT_AND_BACK	= 3
	};

	enum class ECompareOp
	{
		NEVER				= 0,
		LESS				= 1,
		EQUAL				= 2,
		LESS_OR_EQUAL		= 3,
		GREATER				= 4,
		NOT_EQUAL			= 5,
		GREATER_OR_EQUAL	= 6,
		ALWAYS				= 7
	};

	enum class EStencilOp
	{
		KEEP				= 0,
		ZERO				= 1,
		REPLACE				= 2,
		INCREMENT_AND_CLAMP	= 3,
		DECREMENT_AND_CLAMP	= 4,
		INVERT				= 5,
		INCREMENT_AND_WRAP	= 6,
		DECREMENT_AND_WRAP	= 7
	};

	enum class EBlendFactor
	{
		ZERO						= 0,
		ONE							= 1,
		SRC_COLOR					= 2,
		ONE_MINUS_SRC_COLOR			= 3,
		DST_COLOR					= 4,
		ONE_MINUS_DST_COLOR			= 5,
		SRC_ALPHA					= 6,
		ONE_MINUS_SRC_ALPHA			= 7,
		DST_ALPHA					= 8,
		ONE_MINUS_DST_ALPHA			= 9,
		CONSTANT_COLOR				= 10,
		ONE_MINUS_CONSTANT_COLOR	= 11,
		CONSTANT_ALPHA				= 12,
		ONE_MINUS_CONSTANT_ALPHA	= 13,
		SRC_ALPHA_SATURATE			= 14,
		SRC1_COLOR					= 15,
		ONE_MINUS_SRC1_COLOR		= 16,
		SRC1_ALPHA					= 17,
		ONE_MINUS_SRC1_ALPHA		= 18
	};

	enum class EBlendOp
	{
		NONE				= 0,
		ADD					= 1,
		SUBTRACT			= 2,
		REVERSE_SUBTRACT	= 3,
		MIN					= 4,
		MAX					= 5
	};

	enum class FColorComponentFlag
	{
		NONE	= 0,
		RED		= FLAG(1),
		GREEN	= FLAG(2),
		BLUE	= FLAG(3),
		ALPHA	= FLAG(4)
	};
	ENABLE_BITMASK_OPERATORS(FColorComponentFlag);

	enum class ELogicOp
	{
		NONE			= 0,
		CLEAR			= 1,
		AND				= 2,
		AND_REVERSE		= 3,
		COPY			= 4,
		AND_INVERTED	= 5,
		NO_OP			= 6,
		XOR				= 7,
		OR				= 8,
		NOR				= 9,
		EQUIVALENT		= 10,
		INVERT			= 11,
		OR_REVERSE		= 12,
		COPY_INVERTED	= 13,
		OR_INVERTED		= 14,
		NAND			= 15,
		SET				= 16,
	};

	enum class EVertexInputRate
	{
		NONE		= 0,
		VERTEX		= 1,
		INSTANCE	= 2,
	};

	enum class EPipelineType
	{
		NONE		= 0,
		GRAPHICS	= 1,
		COMPUTE		= 2,
		RAY_TRACING	= 3
	};
}