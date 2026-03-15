#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	struct ResourceState
	{
		ETextureLayout	Layout		= ETextureLayout::UNDEFINED;
		FAccessFlag		AccessMask	= FAccessFlag::NONE;
		FPipelineStage	Stage		= FPipelineStage::NONE;
	};
}
