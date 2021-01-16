#include "polypch.h"
#include "PVKTexture.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKTexture::PVKTexture()
	{
	}

	PVKTexture::~PVKTexture()
	{
		PVK_CLEANUP(m_Image, vmaDestroyImage(PVKInstance::GetAllocator(), m_Image, m_Allocation));
		PVK_CLEANUP(m_ImageView, vkDestroyImageView(PVKInstance::GetDevice(), m_ImageView, nullptr));
	}

	void PVKTexture::Init(const TextureDesc* pDesc)
	{
		p_TextureDesc = *pDesc;

		CreateImage();
		CreateImageView();
	}

	void PVKTexture::InitWithImage(const TextureDesc* pDesc, VkImage image)
	{
		p_TextureDesc = *pDesc;

		m_Image = image;
		CreateImageView();
	}

	void PVKTexture::CreateImage()
	{
		VkImageCreateInfo imageInfo = {};
		imageInfo.sType					= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType				= ConvertTextureDimVK(p_TextureDesc.TextureDim);
		imageInfo.extent.width			= p_TextureDesc.Width;
		imageInfo.extent.height			= p_TextureDesc.Height;
		imageInfo.extent.depth			= p_TextureDesc.Depth;
		imageInfo.mipLevels				= p_TextureDesc.MipLevels;
		imageInfo.arrayLayers			= p_TextureDesc.ArrayLayers;
		imageInfo.format				= ConvertFormatVK(p_TextureDesc.Format);
		//If you want to be able to directly access texels in the memory of the image, then you must use VK_IMAGE_TILING_LINEAR
		imageInfo.tiling				= VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout			= VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage					= ConvertTextureUsageVK(p_TextureDesc.TextureUsage);
		imageInfo.samples				= VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		imageInfo.queueFamilyIndexCount	= 1;
		imageInfo.pQueueFamilyIndices	= nullptr;
		imageInfo.flags					= 0;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = ConvertMemoryUsageVMA(p_TextureDesc.MemoryUsage);
		PVK_CHECK(vmaCreateImage(PVKInstance::GetAllocator(), &imageInfo, &allocInfo, &m_Image, &m_Allocation, nullptr), "Failed to create image using VMA!");
	}

	void PVKTexture::CreateImageView()
	{
		VkImageViewCreateInfo createInfo = {};
		createInfo.sType		= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		createInfo.image		= m_Image;
		createInfo.viewType		= ConvertImageViewTypeVK(p_TextureDesc.ImageViewType);
		createInfo.format		= ConvertFormatVK(p_TextureDesc.Format);
		createInfo.components.r	= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.g	= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.b	= VK_COMPONENT_SWIZZLE_IDENTITY;
		createInfo.components.a	= VK_COMPONENT_SWIZZLE_IDENTITY;

		createInfo.subresourceRange.aspectMask = ConvertImageViewFlagsVK(p_TextureDesc.ImageViewFlags);
		createInfo.subresourceRange.baseMipLevel = 0;
		createInfo.subresourceRange.levelCount = 1;
		createInfo.subresourceRange.baseArrayLayer = 0;
		createInfo.subresourceRange.layerCount = 1;

		PVK_CHECK(vkCreateImageView(PVKInstance::GetDevice(), &createInfo, nullptr, &m_ImageView), "Failed to create image view!");
	}

	uint64 PVKTexture::GetNative() const
	{
		return reinterpret_cast<uint64>(m_Image);
	}
}