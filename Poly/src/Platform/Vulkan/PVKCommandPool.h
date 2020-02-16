#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Poly
{
	class PVKInstance;
	class PVKCommandBuffer;

	class PVKCommandPool
	{
	public:
		PVKCommandPool();
		~PVKCommandPool();

		void init(PVKInstance* instance, VkQueueFlagBits queueType);
		void cleanup();

		VkCommandPool getCommandPool() const { return this->pool; }
		VkQueue getQueue();

		PVKCommandBuffer* beginSingleTimeCommand();
		void endSingleTimeCommand(PVKCommandBuffer* buffer);

		PVKCommandBuffer* createCommandBuffer();
		std::vector<PVKCommandBuffer*> createCommandBuffers(uint32_t count);
		void removeCommandBuffer(PVKCommandBuffer* buffer);

	private:
		void createCommandPool();

		VkCommandPool pool;
		VkQueueFlagBits queue;

		PVKInstance* instance;
		std::vector<PVKCommandBuffer*> buffers;
	};
}