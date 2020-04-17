#include "polypch.h"
#include "PVKImageView.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKImageView::PVKImageView() : imageView(VK_NULL_HANDLE)
	{
	}
	void PVKImageView::init(VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectMask, uint32_t layerCount)
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image = image;

		createInfo.viewType = type;
		createInfo.format = format;

		createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = aspectMask;
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		PVK_CHECK(vkCreateImageView(PVKInstance::getDevice(), &createInfo, nullptr, &this->imageView), "Failed to create image view!");
	}

	void PVKImageView::cleanup()
	{
		vkDestroyImageView(PVKInstance::getDevice(), this->imageView, nullptr);
	}

}