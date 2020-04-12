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

		void init(PVKSwapChain& swapChain);
		void cleanup();

		void addSubpassDependency(VkSubpassDependency dep);

		VkRenderPass getNative() { return this->renderPass; };

	private:
		VkFormat imageFormat;
		VkRenderPass renderPass;

		std::vector<VkSubpassDependency> subpassDependencies;
	};

}