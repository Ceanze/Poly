#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKSwapChain;
	class PVKRenderPass;

	class PVKFramebuffer
	{
	public:
		PVKFramebuffer();
		~PVKFramebuffer();

		void init(PVKSwapChain& swapChain, PVKRenderPass& renderPass, VkImageView attachment);
		void cleanup();

		VkFramebuffer getNative() const { return this->framebuffer; }

	private:
		VkFramebuffer framebuffer;
	};
}