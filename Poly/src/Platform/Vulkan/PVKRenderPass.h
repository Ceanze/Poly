#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{

	class PVKRenderPass
	{
	public:
		PVKRenderPass();
		~PVKRenderPass();

		void init(VkDevice device, VkFormat imageFormat);
		void cleanup();

		VkRenderPass getRenderPass() { return this->renderPass; };

	private:
		VkDevice device;
		VkFormat imageFormat;
		VkRenderPass renderPass;
	};

}