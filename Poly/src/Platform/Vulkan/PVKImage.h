#pragma once

#include "PVKTypes.h"

#include "VmaInclude.h"

namespace Poly
{

	class PVKCommandPool;
	class PVKBuffer;

	class PVKImage
	{
	public:
		PVKImage();
		~PVKImage() = default;

		void Init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueFamilyIndex, VmaMemoryUsage memoryUsage);
		void Init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueFamilyIndices, VmaMemoryUsage memoryUsage);
		void Cleanup();

		void CopyBufferToImage(PVKBuffer& buffer, PVKCommandPool* pool);
		void CopyBufferToImage(PVKBuffer& buffer, PVKCommandPool* pool, const std::vector<VkBufferImageCopy>& regions);
		void TransitionLayout(ColorFormat format, ImageLayout oldLayout, ImageLayout newLayout, PVKCommandPool* pool, uint32_t layerCount = 1);

		VkImage GetNative() const { return m_Image; }
		VkImageLayout GetLayout() const { return m_Layout; }


	private:
		VkImage			m_Image			= VK_NULL_HANDLE;
		VkImageLayout	m_Layout		= VK_IMAGE_LAYOUT_UNDEFINED;
		VmaAllocation	m_Allocation	= VK_NULL_HANDLE;

		uint32_t m_Width = 0;
		uint32_t m_Height = 0;
	};

}