#include "polypch.h"
#include "PVKSampler.h"

#include "PVKInstance.h"

namespace Poly
{

	PVKSampler::PVKSampler()
	{
		createNativeSampler(Sampler::Filter::LINEAR, Sampler::AddressMode::REPEAT, Sampler::Filter::LINEAR);
	}

	PVKSampler::PVKSampler(Sampler::Filter filter, Sampler::AddressMode addressMode, Sampler::Filter mipmap)
	{
		createNativeSampler(filter, addressMode, mipmap);
	}

	PVKSampler::~PVKSampler()
	{
		PVK_CLEANUP(this->sampler, vkDestroySampler(PVKInstance::getDevice(), this->sampler, nullptr));
	}

	void PVKSampler::createNativeSampler(Sampler::Filter filter, Sampler::AddressMode addressMode, Sampler::Filter mipmap)
	{
		if (mipmap == Sampler::Filter::CUBIC_IMG) {
			POLY_CORE_ERROR("Mipmap cannot be CUBIC_IMG for a sampler!");
			POLY_CORE_INFO("Setting mipmap to LINEAR for sampler");
		}

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;

		// Editable settings
		samplerInfo.magFilter = (VkFilter)filter;
		samplerInfo.minFilter = (VkFilter)filter;
		samplerInfo.addressModeU = (VkSamplerAddressMode)addressMode;
		samplerInfo.addressModeV = (VkSamplerAddressMode)addressMode;
		samplerInfo.addressModeW = (VkSamplerAddressMode)addressMode;
		samplerInfo.mipmapMode = (VkSamplerMipmapMode)mipmap;

		// Currently NOT editable settings
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		samplerInfo.anisotropyEnable = VK_TRUE;
		samplerInfo.maxAnisotropy = 16.0f;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;

		PVK_CHECK(vkCreateSampler(PVKInstance::getDevice(), &samplerInfo, nullptr, &this->sampler), "Failed to create sampler!");
	}

}