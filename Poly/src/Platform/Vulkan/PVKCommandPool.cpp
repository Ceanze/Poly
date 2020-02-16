#include "polypch.h"
#include "PVKCommandPool.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"
#include "PVKCommandBuffer.h"

namespace Poly
{

	PVKCommandPool::PVKCommandPool() :
		pool(VK_NULL_HANDLE), instance(nullptr), queue(VK_QUEUE_GRAPHICS_BIT)
	{
	}

	PVKCommandPool::~PVKCommandPool()
	{
	}

	void PVKCommandPool::init(PVKInstance* instance, VkQueueFlagBits queueType)
	{
		this->instance = instance;
		
		createCommandPool();
	}

	void PVKCommandPool::cleanup()
	{
		vkDestroyCommandPool(this->instance->getDevice(), this->pool, nullptr);

		// Command buffers are automatically freed when command pool is destroyed, just cleans memory
		for (auto buffer : this->buffers) {
			buffer->cleanup();
			delete buffer;
		}
		this->buffers.clear();
	}

	VkQueue PVKCommandPool::getQueue()
	{
		switch (this->queue) {
		case VK_QUEUE_GRAPHICS_BIT:
			return this->instance->getGraphicsQueue().queue;
		default:
			POLY_ERROR("Could not find queue flag with bits {}!", this->queue);
		}
	}

	PVKCommandBuffer* PVKCommandPool::beginSingleTimeCommand()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = this->pool;
		allocInfo.commandBufferCount = 1;

		PVKCommandBuffer* buffer = createCommandBuffer();

		buffer->begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		return buffer;
	}

	void PVKCommandPool::endSingleTimeCommand(PVKCommandBuffer* buffer)
	{
		buffer->end();

		VkCommandBuffer commandBuffer = buffer->getCommandBuffer();
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(getQueue(), 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(getQueue());

		vkFreeCommandBuffers(this->instance->getDevice(), this->pool, 1, &commandBuffer);
		removeCommandBuffer(buffer);
	}

	PVKCommandBuffer* PVKCommandPool::createCommandBuffer()
	{
		PVKCommandBuffer* b = new PVKCommandBuffer();
		b->init(this->instance, this->pool);
		b->createCommandBuffer();
		buffers.push_back(b);
		return b;
	}

	std::vector<PVKCommandBuffer*> PVKCommandPool::createCommandBuffers(uint32_t count)
	{
		std::vector<VkCommandBuffer> vkBuffers(count);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = this->pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = count;

		PVK_CHECK(vkAllocateCommandBuffers(this->instance->getDevice(), &allocInfo, vkBuffers.data()), "Failed to allocate command buffers!")

		std::vector<PVKCommandBuffer*> b;
		for (size_t i = 0; i < count; i++) {
			b[i] = new PVKCommandBuffer;
			b[i]->init(this->instance, this->pool);
			b[i]->setCommandBuffer(vkBuffers[i]);
			this->buffers.push_back(b[i]);
		}

		return b;
	}

	void PVKCommandPool::removeCommandBuffer(PVKCommandBuffer* buffer)
	{
		buffers.erase(std::remove(buffers.begin(), buffers.end(), buffer), buffers.end());
		delete buffer;
	}

	void PVKCommandPool::createCommandPool()
	{
		// Note: A command pool only supports one queue type at a time, for more queues more command pools are needed

		uint32_t queueIndex = findQueueIndex(this->queue, this->instance->getPhysicalDevice());

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueIndex;
		poolInfo.flags = 0; // Optional

		PVK_CHECK(vkCreateCommandPool(this->instance->getDevice(), &poolInfo, nullptr, &this->pool), "Failed to create command pool!");
	}

}