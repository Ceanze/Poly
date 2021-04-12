#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Sampler;

	using PassResourcePair = std::pair<std::string, std::string>;

	/**
	 * Specifies default values for things like texture size
	 * and other common values
	 */
	struct RenderGraphDefaultParams
	{
		uint32 TextureWidth			= 0;
		uint32 TextureHeight		= 0;
		uint32 MaxBackbufferCount	= 3;
		Ref<Sampler> pSampler		= nullptr;
	};
}