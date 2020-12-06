#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// TODO: Add support for several subpasses and color attachments

namespace Poly
{

	class PVKSwapChain;

	class PVKRenderPass
	{
	public:
		PVKRenderPass();
		~PVKRenderPass();

		void Init(PVKSwapChain& swapChain);
		void Cleanup();

		void AddSubpassDependency(VkSubpassDependency dep);

		VkRenderPass GetNative() { return m_RenderPass; };

	private:
		VkFormat m_ImageFormat		= VK_FORMAT_UNDEFINED;
		VkRenderPass m_RenderPass		= VK_NULL_HANDLE;

		std::vector<VkSubpassDependency> m_SubpassDependencies;
	};

}