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

		VkSampler getNative() const { return this->sampler; }

	private:
		void createNativeSampler(Sampler::Filter filter, Sampler::AddressMode addressMode, Sampler::Filter mipmap);

		VkSampler sampler = VK_NULL_HANDLE;
	};
}