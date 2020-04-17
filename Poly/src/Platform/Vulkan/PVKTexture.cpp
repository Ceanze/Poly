#include "polypch.h"
#include "PVKTexture.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKTexture::PVKTexture() : width(0), height(0), format(VK_FORMAT_UNDEFINED)
	{
	}

	void PVKTexture::init(uint32_t width, uint32_t height, VkFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueIndices)
	{
		init(width, height, format, usage, flags, arrayLayers, { queueIndices });
	}

	void PVKTexture::init(uint32_t width, uint32_t height, VkFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueIndices)
	{
		this->width = width;
		this->height = height;
		this->format = format;
		this->image.init(width, height, format, usage, flags, arrayLayers, queueIndices);
	}

	void PVKTexture::initView(VkImageViewType type, VkImageAspectFlags aspect)
	{
		this->imageView.init(image.getNative(), type, this->format, aspect);
	}

	void PVKTexture::cleanup()
	{
		this->image.cleanup();
		this->imageView.cleanup();
	}

	VkMemoryRequirements PVKTexture::getMemoryRequirements() const
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(PVKInstance::getDevice(), this->image.getNative(), &memRequirements);

		return memRequirements;
	}

}