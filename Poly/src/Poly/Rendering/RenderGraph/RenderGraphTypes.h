#pragma once

#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Sampler;
	class Resource;

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
		uint32 TextureDepth			= 1;
		uint32 MaxBackbufferCount	= 3;
		Ref<Sampler> pSampler		= nullptr;
		EFormat Format				= EFormat::UNDEFINED;
	};

	struct ResourceInfo
	{
		Ref<Resource>	pResource			= nullptr;
		bool			AutoBindDescriptor	= true;
	};
}