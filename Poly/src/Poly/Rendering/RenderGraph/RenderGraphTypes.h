#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Sampler;

	using PassResourcePair = std::pair<std::string, std::string>;

	inline PassResourcePair GetPassResourcePair(std::string name)
	{
		auto pos = name.find_first_of('.');

		// If no dot was found - then only a pass name was given
		if (pos == std::string::npos)
			return { "", name };

		return { name.substr(0, pos), name.substr(pos + 1) };
	}

	inline FTextureUsage ConvertResourceBindPointToTextureUsage(FResourceBindPoint bindPoint)
	{
		FTextureUsage mask = FTextureUsage::NONE;
		if (BitsSet(FResourceBindPoint::COLOR_ATTACHMENT, bindPoint)) mask |= FTextureUsage::COLOR_ATTACHMENT;
		if (BitsSet(FResourceBindPoint::DEPTH_STENCIL, bindPoint)) mask |= FTextureUsage::DEPTH_STENCIL_ATTACHMENT;
		if (BitsSet(FResourceBindPoint::SAMPLER, bindPoint)) mask |= FTextureUsage::SAMPLED;
		if (BitsSet(FResourceBindPoint::SHADER_READ, bindPoint)) mask |= FTextureUsage::SHADER_RESOURCE;
		return mask;
	}

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