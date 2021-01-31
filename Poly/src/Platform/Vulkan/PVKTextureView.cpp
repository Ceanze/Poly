#include "polypch.h"
#include "PVKTextureView.h"

#include "PVKTexture.h"
#include "PVKInstance.h"

namespace Poly
{
	PVKTextureView::~PVKTextureView()
	{
		vkDestroyImageView(PVKInstance::GetDevice(), m_ImageView, nullptr);
	}

	void PVKTextureView::Init(const TextureViewDesc* pDesc)
	{
		p_TextureViewDesc = *pDesc;
		PVKTexture* pTexture = reinterpret_cast<PVKTexture*>(pDesc->pTexture);

		VkImageViewCreateInfo createInfo = {};
		createInfo.sType	= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.pNext	= nullptr;
		createInfo.image	= pTexture ? pTexture->GetImageVK() : nullptr;
		createInfo.viewType	= ConvertImageViewTypeVK(pDesc->ImageViewType);
		createInfo.format	= ConvertFormatVK(pDesc->Format);
		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY; // Current implementation assume these are always correct
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.subresourceRange.aspectMask		= ConvertImageViewFlagsVK(pDesc->ImageViewFlag);
		createInfo.subresourceRange.baseMipLevel	= pDesc->MipLevel;
		createInfo.subresourceRange.levelCount		= pDesc->MipLevelCount;
		createInfo.subresourceRange.baseArrayLayer	= pDesc->ArrayLayer;
		createInfo.subresourceRange.layerCount		= pDesc->ArrayLayerCount;

		PVK_CHECK(vkCreateImageView(PVKInstance::GetDevice(), &createInfo, nullptr, &m_ImageView), "Failed to create image view!");
	}
}