#include "polypch.h"
#include "PVKCommandPool.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"
#include "PVKCommandBuffer.h"

namespace Poly
{

	PVKCommandPool::PVKCommandPool() :
		pool(VK_NULL_HANDLE), queue(QueueType::GRAPHICS)
	{
	}

	PVKCommandPool::~PVKCommandPool()
	{
	}

	void PVKCommandPool::init(QueueType queueType)
	{
		this->queue = queueType;
		createCommandPool();
	}

	void PVKCommandPool::cleanup()
	{
		vkDestroyCommandPool(PVKInstance::getDevice(), this->pool, nullptr);

		// Command buffers are automatically freed when command pool is destroyed, just cleans memory
		for (auto buffer : this->buffers) {
			buffer->cleanup();
			delete buffer;
		}
		this->buffers.clear();
	}

	QueueType PVKCommandPool::getQueueType() const
	{
		return this->queue;
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

		VkCommandBuffer commandBuffer = buffer->getNative();
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(PVKInstance::getQueue(this->queue).queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(PVKInstance::getQueue(this->queue).queue);

		vkFreeCommandBuffers(PVKInstance::getDevice(), this->pool, 1, &commandBuffer);
		removeCommandBuffer(buffer);
	}

	PVKCommandBuffer* PVKCommandPool::createCommandBuffer()
	{
		PVKCommandBuffer* b = new PVKCommandBuffer();
		b->init(this->pool);
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

		PVK_CHECK(vkAllocateCommandBuffers(PVKInstance::getDevice(), &allocInfo, vkBuffers.data()), "Failed to allocate command buffers!")

		std::vector<PVKCommandBuffer*> b(count);
		for (size_t i = 0; i < count; i++) {
			b[i] = new PVKCommandBuffer;
			b[i]->init(this->pool);
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

		uint32_t queueIndex = PVKInstance::getQueue(this->queue).queueIndex;

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueIndex;
		poolInfo.flags = 0; // Optional

		PVK_CHECK(vkCreateCommandPool(PVKInstance::getDevice(), &poolInfo, nullptr, &this->pool), "Failed to create command pool!");
	}

}