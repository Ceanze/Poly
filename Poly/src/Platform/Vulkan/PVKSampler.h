#pragma once

#include <vulkan/vulkan.h>
#include "Poly/Rendering/Sampler.h"

namespace Poly
{
	class PVKSampler : public Sampler
	{
	public:
		PVKSampler();
		PVKSampler(Sampler::Filter filter, Sampler::AddressMode addressMode, Sampler::Filter mipmap);
		virtual ~PVKSampler();

		VkSampler GetNative() const { return m_Sampler; }

	private:
		void CreateNativeSampler(Sampler::Filter filter, Sampler::AddressMode addressMode, Sampler::Filter mipmap);

		VkSampler m_Sampler = VK_NULL_HANDLE;
	};
}