#pragma once

#include <vulkan/vulkan.h>
#include "PVKTypes.h"

namespace Poly
{

	class PVKImageView
	{
	public:
		PVKImageView();
		~PVKImageView() = default;

		void init(VkImage image, VkImageViewType type, ColorFormat format, VkImageAspectFlags aspectMask, uint32_t layerCount = 1);
		void cleanup();

		VkImageView getNative() const { return this->imageView; };

	private:
		VkImageView imageView;
	};

}