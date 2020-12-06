#include "polypch.h"
#include "PVKImageView.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKImageView::PVKImageView()
	{
	}
	void PVKImageView::Init(VkImage image, ImageViewType type, ColorFormat format, ImageAspect aspectMask, uint32_t layerCount)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = (VkImageViewType)type;
		createInfo.format = (VkFormat)format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = (VkImageAspectFlags)aspectMask;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		PVK_CHECK(vkCreateImageView(PVKInstance::GetDevice(), &createInfo, nullptr, &m_ImageView), "Failed to create image view!");
	}

	void PVKImageView::Cleanup()
	{
		PVK_CLEANUP(m_ImageView, vkDestroyImageView(PVKInstance::GetDevice(), m_ImageView, nullptr));
	}

}