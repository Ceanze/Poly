#pragma once

#include "PVKTypes.h"

#include <vulkan/vulkan.h>

namespace Poly
{

	class PVKCommandPool;
	class PVKBuffer;

	class PVKImage
	{
	public:
		PVKImage();
		~PVKImage() = default;

		void init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueFamilyIndex);
		void init(uint32_t width, uint32_t height, ColorFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueFamilyIndices);
		void cleanup();

		void copyBufferToImage(PVKBuffer& buffer, PVKCommandPool* pool);
		void copyBufferToImage(PVKBuffer& buffer, PVKCommandPool* pool, const std::vector<VkBufferImageCopy>& regions);
		void transitionLayout(ColorFormat format, ImageLayout oldLayout, ImageLayout newLayout, PVKCommandPool* pool, uint32_t layerCount = 1);

		VkImage getNative() const { return this->image; }
		VkImageLayout getLayout() const { return this->layout; }


	private:
		VkImage image;
		VkImageLayout layout;

		uint32_t width, height;
	};

}