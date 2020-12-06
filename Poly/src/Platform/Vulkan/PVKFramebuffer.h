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

		void Init(PVKSwapChain& swapChain, PVKRenderPass& renderPass, VkImageView attachment);
		void Cleanup();

		VkFramebuffer GetNative() const { return m_Framebuffer; }

	private:
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
	};
}