#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Sampler;

	using PassResourcePair = std::pair<std::string, std::string>;
	using GroupResourcePair = std::pair<std::string, std::string>;

	inline std::pair<std::string, std::string> SeparateStrings(const std::string& value, char separator)
	{
		auto pos = value.find_first_of(separator);

		if (pos == std::string::npos)
			return { "", value };

		return { value.substr(0, pos), value.substr(pos + 1) };
	}

	inline PassResourcePair GetPassResourcePair(const std::string& name)
	{
		return SeparateStrings(name, '.');
	}

	inline GroupResourcePair GetGroupResourcePair(const std::string& name)
	{
		return SeparateStrings(name, ':');
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