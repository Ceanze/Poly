#include "polypch.h"
#include "PVKTexture.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKTexture::PVKTexture() : width(0), height(0), format(ColorFormat::UNDEFINED)
	{
	}

	void PVKTexture::init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueIndices)
	{
		init(width, height, format, usage, flags, arrayLayers, { queueIndices });
	}

	void PVKTexture::init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueIndices)
	{
		this->width = width;
		this->height = height;
		this->format = format;
		this->image.init(width, height, format, usage, flags, arrayLayers, queueIndices);
	}

	void PVKTexture::initView(ImageViewType type, ImageAspect aspect)
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

	PVKTexture2D::PVKTexture2D(uint32_t width, uint32_t height) :
		width(width), height(height)
	{
		// TODO: Make it possible to change usage and format
		// Current implementation assumes a R8G8B8A8_UNORM format with a sampled usage
		this->image.init(width, height, ColorFormat::R8G8B8A8_UNORM, ImageUsage::SAMPLED, ImageCreate::NONE, 1, PVKInstance::getQueue(QueueType::GRAPHICS).queueIndex);
		this->imageView.init(this->image.getNative(), ImageViewType::DIM_2, ColorFormat::R8G8B8A8_UNORM, ImageAspect::COLOR_BIT);
	}

	PVKTexture2D::PVKTexture2D(const std::string& path) :
		path(path), width(0), height(0)
	{
		// TODO: Read from file and create texture - need stbi
	}

	PVKTexture2D::~PVKTexture2D()
	{
		this->image.cleanup();
		this->imageView.cleanup();
	}

	void PVKTexture2D::setData(void* data, uint32_t size)
	{
	}

}