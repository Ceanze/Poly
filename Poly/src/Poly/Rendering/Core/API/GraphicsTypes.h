#pragma once

/*
	This file defines all of the types that is commonly used for all rendering platforms
*/

#include "Poly/Core/PolyUtils.h"

namespace Poly
{
	enum class FShaderStage {
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

	enum class EMemoryUsage {
		UNKNOWN				= 0,
		GPU_ONLY			= 1,
		CPU_VISIBLE			= 2,
		CPU_GPU_MAPPABLE	= 3 // Prefer to only use this for debug
	};

	enum class FBufferUsage {
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

	enum class EFormat {
		UNDEFINED			= 0,
		R8G8B8A8_UNORM		= 1,
		D24_UNORM_S8_UINT	= 2
	};

	enum class FTextureUsage {
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

	enum class ETextureDim {
		NONE	= 0,
		DIM_1D	= 1,
		DIM_2D	= 2,
		DIM_3D	= 3,
	};

	enum class EImageViewType {
		NONE			= 0,
		TYPE_1D			= 1,
		TYPE_2D 		= 2,
		TYPE_3D			= 3,
		TYPE_CUBE		= 4,
		TYPE_CUBE_ARRAY	= 5,
		TYPE_2D_ARRAY	= 6,
		TYPE_1D_ARRAY	= 7
	};

	enum class FImageViewFlag {
		NONE		= 0,
		SHADER_RESOURCE		= FLAG(1),
		RENDER_TARGET		= SHADER_RESOURCE,
		UNORDERED_ACCESS	= SHADER_RESOURCE,
		DEPTH_STENCIL		= FLAG(2)
	};
	ENABLE_BITMASK_OPERATORS(FImageViewFlag);
}