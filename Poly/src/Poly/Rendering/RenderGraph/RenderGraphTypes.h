#pragma once

namespace Poly
{
	using PassResourcePair = std::pair<std::string, std::string>;

	/**
	 * Specifies default values for things like texture size
	 * and other common values
	 */
	struct RenderGraphDefaultParams
	{
		uint32 TextureWidth		= 0;
		uint32 TextureHeight	= 0;
	};
}