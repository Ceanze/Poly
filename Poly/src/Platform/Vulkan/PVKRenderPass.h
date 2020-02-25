#pragma once

#include <vulkan/vulkan.h>
#include <vector>

// TODO: Add support for several subpasses and color attachments

namespace Poly
{

	class PVKInstance;
	class PVKSwapChain;

	class PVKRenderPass
	{
	public:
		PVKRenderPass();
		~PVKRenderPass();

		void init(PVKInstance* instance, PVKSwapChain* swapChain);
		void cleanup();

		void addSubpassDependency(VkSubpassDependency dep);

		VkRenderPass getRenderPass() { return this->renderPass; };

	private:
		VkDevice device;
		VkFormat imageFormat;
		VkRenderPass renderPass;

		std::vector<VkSubpassDependency> subpassDependencies;
	};

}