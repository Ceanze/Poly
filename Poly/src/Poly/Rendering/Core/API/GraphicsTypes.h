#pragma once

/*
	This file defines all of the types that is commonly used for all rendering platforms
*/

#include "Poly/Core/PolyUtils.h"

namespace Poly
{
	enum class ShaderStage {
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
	ENABLE_BITMASK_OPERATORS(ShaderStage);
}