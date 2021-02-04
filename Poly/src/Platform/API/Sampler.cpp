#include "polypch.h"
#include "Sampler.h"

#include "Poly/Core/RenderAPI.h"

namespace Poly
{
	void Sampler::InitDefaults()
	{
		SamplerDesc samplerDesc = {};
		samplerDesc.MinFilter		= EFilter::NEAREST;
		samplerDesc.MagFilter		= EFilter::NEAREST;
		samplerDesc.AddressModeU	= ESamplerAddressMode::REPEAT;
		samplerDesc.AddressModeV	= ESamplerAddressMode::REPEAT;
		samplerDesc.AddressModeW	= ESamplerAddressMode::REPEAT;
		samplerDesc.MipMapMode		= ESamplerMipmapMode::NEAREST;
		samplerDesc.BorderColor		= EBorderColor::INT_OPAQUE_BLACK;
		samplerDesc.MipLodBias		= 0.0f;
		samplerDesc.MinLod			= 0.0f;
		samplerDesc.MaxLod			= 0.0f;
		samplerDesc.AnistropyEnable	= true;
		samplerDesc.MaxAnisotropy	= 16.0f;

		s_pNearestSampler = RenderAPI::CreateSampler(&samplerDesc);

		samplerDesc.MinFilter		= EFilter::LINEAR;
		samplerDesc.MagFilter		= EFilter::LINEAR;
		samplerDesc.MipMapMode		= ESamplerMipmapMode::LINEAR;

		s_pLinearSampler = RenderAPI::CreateSampler(&samplerDesc);
	}

	void Sampler::CleanupDefaults()
	{
		s_pNearestSampler.reset();
		s_pLinearSampler.reset();
	}
}