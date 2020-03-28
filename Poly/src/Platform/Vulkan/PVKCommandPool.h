#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace Poly
{
	class PVKCommandBuffer;

	class PVKCommandPool
	{
	public:
		PVKCommandPool();
		~PVKCommandPool();

		void init(VkQueueFlagBits queueType);
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

		std::vector<PVKCommandBuffer*> buffers;
	};
}