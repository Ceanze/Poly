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

	private:
		VkFramebuffer framebuffer;

		PVKInstance* instance;
	};
}