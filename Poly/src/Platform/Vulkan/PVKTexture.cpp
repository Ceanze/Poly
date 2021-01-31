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
		if (m_HandleImage)
			PVK_CLEANUP(m_Image, vmaDestroyImage(PVKInstance::GetAllocator(), m_Image, m_Allocation));
	}

	void PVKTexture::Init(const TextureDesc* pDesc)
	{
		p_TextureDesc = *pDesc;
		m_HandleImage = true;

		CreateImage();
	}

	void PVKTexture::InitWithImage(const TextureDesc* pDesc, VkImage image)
	{
		p_TextureDesc = *pDesc;
		m_HandleImage = false;
		m_Image = image;
	}

	void PVKTexture::CreateImage()
	{
		VkSampleCountFlagBits sampleCount = ConvertSampleCountVK(p_TextureDesc.SampleCount);

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
		imageInfo.samples				= sampleCount;
		imageInfo.sharingMode			= VK_SHARING_MODE_EXCLUSIVE; // At the moment only one queue ownership is allowed
		imageInfo.queueFamilyIndexCount	= 1;
		imageInfo.pQueueFamilyIndices	= nullptr;
		imageInfo.flags					= 0;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = ConvertMemoryUsageVMA(p_TextureDesc.MemoryUsage);
		PVK_CHECK(vmaCreateImage(PVKInstance::GetAllocator(), &imageInfo, &allocInfo, &m_Image, &m_Allocation, nullptr), "Failed to create image using VMA!");
	}
}