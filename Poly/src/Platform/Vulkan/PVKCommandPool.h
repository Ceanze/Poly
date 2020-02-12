#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKInstance;

	class PVKCommandPool
	{
	public:
		PVKCommandPool();
		~PVKCommandPool();

		void init(PVKInstance* instance);
		void cleanup();

		VkCommandPool getCommandPool() const { return this->commandPool; }

	private:
		void createCommandPool();

		VkCommandPool commandPool;

		PVKInstance* instance;
	};
}