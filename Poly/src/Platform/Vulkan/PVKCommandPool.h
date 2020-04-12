#pragma once

#include "PVKTypes.h"

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

		void init(QueueType queueType);
		void cleanup();

		VkCommandPool getNative() const { return this->pool; }
		QueueType getQueueType() const;

		PVKCommandBuffer* beginSingleTimeCommand();
		void endSingleTimeCommand(PVKCommandBuffer* buffer);

		PVKCommandBuffer* createCommandBuffer();
		std::vector<PVKCommandBuffer*> createCommandBuffers(uint32_t count);
		void removeCommandBuffer(PVKCommandBuffer* buffer);

	private:
		void createCommandPool();

		VkCommandPool pool;
		QueueType queue;

		std::vector<PVKCommandBuffer*> buffers;
	};
}