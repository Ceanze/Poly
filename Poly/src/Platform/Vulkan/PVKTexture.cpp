#include "polypch.h"
#include "PVKTexture.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKTexture::PVKTexture()
	{
	}

	void PVKTexture::Init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueIndices, VmaMemoryUsage memoryUsage)
	{
		std::vector<uint32_t> vec = { queueIndices };
		Init(width, height, format, usage, flags, arrayLayers, vec, memoryUsage);
	}

	void PVKTexture::Init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueIndices, VmaMemoryUsage memoryUsage)
	{
		m_Width = width;
		m_Height = height;
		m_Format = format;
		m_Image.Init(width, height, format, usage, flags, arrayLayers, queueIndices, memoryUsage);
	}

	void PVKTexture::InitView(ImageViewType type, ImageAspect aspect)
	{
		m_ImageView.Init(m_Image.GetNative(), type, m_Format, aspect);
	}

	void PVKTexture::Cleanup()
	{
		m_Image.Cleanup();
		m_ImageView.Cleanup();
	}

	VkMemoryRequirements PVKTexture::GetMemoryRequirements() const
	{
		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(PVKInstance::GetDevice(), m_Image.GetNative(), &memRequirements);

		return memRequirements;
	}

	PVKTexture2D::PVKTexture2D(uint32_t width, uint32_t height)
	{
		// TODO: Make it possible to change usage and format
		// Current implementation assumes a R8G8B8A8_UNORM format with a sampled usage
		m_Image.Init(width, height, ColorFormat::R8G8B8A8_UNORM, ImageUsage::SAMPLED, ImageCreate::NONE, 1, PVKInstance::GetQueue(QueueType::GRAPHICS).queueIndex, VMA_MEMORY_USAGE_GPU_ONLY);
		m_ImageView.Init(m_Image.GetNative(), ImageViewType::DIM_2, ColorFormat::R8G8B8A8_UNORM, ImageAspect::COLOR_BIT);
	}

	PVKTexture2D::PVKTexture2D(const std::string& path)
	{
		// TODO: Read from file and create texture - need stbi
		m_Path = path;
	}

	PVKTexture2D::~PVKTexture2D()
	{
		m_Image.Cleanup();
		m_ImageView.Cleanup();
	}

	void PVKTexture2D::SetData(void* data, uint32_t size)
	{
	}

}