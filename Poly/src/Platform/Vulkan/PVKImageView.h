#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{

	class PVKImageView
	{
	public:
		PVKImageView();
		~PVKImageView() = default;

		void init(VkImage image, VkImageViewType type, VkFormat format, VkImageAspectFlags aspectMask, uint32_t layerCount = 1);
		void cleanup();

		VkImageView getNative() const { return this->imageView; };

	private:
		VkImageView imageView;
	};

}