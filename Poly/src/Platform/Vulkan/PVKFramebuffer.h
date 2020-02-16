#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKSwapChain;
	class PVKRenderPass;
	class PVKInstance;

	class PVKFramebuffer
	{
	public:
		PVKFramebuffer();
		~PVKFramebuffer();

		void init(PVKInstance* instance, PVKSwapChain* swapChain, PVKRenderPass* renderPass, VkImageView attachment);
		void cleanup();

		VkFramebuffer getFramebuffer() const { return this->framebuffer; }

	private:
		VkFramebuffer framebuffer;

		PVKInstance* instance;
	};
}