#include "polypch.h"
#include "PVKSampler.h"

#include "PVKInstance.h"

namespace Poly
{

	PVKSampler::~PVKSampler()
	{
		PVK_CLEANUP(m_Sampler, vkDestroySampler(PVKInstance::GetDevice(), m_Sampler, nullptr));
	}

	void PVKSampler::Init(const SamplerDesc* pDesc)
	{
		p_SamplerDesc = *pDesc;

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter				= ConvertFilterVK(pDesc->MagFilter);
		samplerInfo.minFilter				= ConvertFilterVK(pDesc->MinFilter);
		samplerInfo.addressModeU			= ConvertSamplerAddressModeVK(pDesc->AddressModeU);
		samplerInfo.addressModeV			= ConvertSamplerAddressModeVK(pDesc->AddressModeV);
		samplerInfo.addressModeW			= ConvertSamplerAddressModeVK(pDesc->AddressModeW);
		samplerInfo.mipmapMode				= ConvertSamplerMipmapModeVK(pDesc->MipMapMode);
		samplerInfo.mipLodBias				= pDesc->MipLodBias;
		samplerInfo.minLod					= pDesc->MinLod;
		samplerInfo.maxLod					= pDesc->MaxLod;
		samplerInfo.anisotropyEnable		= pDesc->AnistropyEnable;
		samplerInfo.maxAnisotropy			= pDesc->MaxAnisotropy;
		samplerInfo.borderColor				= ConvertBorderColorVK(pDesc->BorderColor);
		samplerInfo.unnormalizedCoordinates	= VK_FALSE;
		samplerInfo.compareEnable			= VK_FALSE;
		samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;

		PVK_CHECK(vkCreateSampler(PVKInstance::GetDevice(), &samplerInfo, nullptr, &m_Sampler), "Failed to create sampler!");
	}
}