#pragma once

#include "PVKTypes.h"
#include "polypch.h"
#include "PVKImage.h"
#include "PVKImageView.h"

#include <vulkan/vulkan.h>

namespace Poly
{

	class PVKTexture
	{
	public:
		PVKTexture();
		~PVKTexture() = default;

		void init(uint32_t width, uint32_t height, VkFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, uint32_t queueIndices);
		void init(uint32_t width, uint32_t height, VkFormat format, ImageUsage usage, ImageCreate flags, uint32_t arrayLayers, const std::vector<uint32_t>& queueIndices);
		void initView(VkImageViewType type, VkImageAspectFlags aspect);
		void cleanup();

		uint32_t getWidth() const { return this->width; }
		uint32_t getHeight() const { return this->height; }
		VkFormat getFormat() const { return this->format; }
		PVKImage& getImage() { return this->image; }
		PVKImageView& getImageView() { return this->imageView; }
		VkMemoryRequirements getMemoryRequirements() const;

	private:
		uint32_t width, height;
		PVKImage image;
		PVKImageView imageView;
		VkFormat format;
	};

}