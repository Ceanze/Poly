#include "polypch.h"
#include "PVKCommandPool.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"
#include "PVKCommandBuffer.h"

namespace Poly
{

	PVKCommandPool::PVKCommandPool()
	{
	}

	PVKCommandPool::~PVKCommandPool()
	{
	}

	void PVKCommandPool::Init(QueueType queueType)
	{
		m_Queue = queueType;
		createCommandPool();
	}

	void PVKCommandPool::Cleanup()
	{
		vkDestroyCommandPool(PVKInstance::GetDevice(), m_Pool, nullptr);

		// Command buffers are automatically freed when command pool is destroyed, just cleans memory
		for (auto buffer : m_Buffers) {
			buffer->Cleanup();
			delete buffer;
		}
		m_Buffers.clear();
	}

	QueueType PVKCommandPool::GetQueueType() const
	{
		return m_Queue;
	}

	PVKCommandBuffer* PVKCommandPool::BeginSingleTimeCommand()
	{
		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandPool = m_Pool;
		allocInfo.commandBufferCount = 1;

		PVKCommandBuffer* buffer = CreateCommandBuffer();

		buffer->Begin(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);

		return buffer;
	}

	void PVKCommandPool::EndSingleTimeCommand(PVKCommandBuffer* pBuffer)
	{
		pBuffer->End();

		VkCommandBuffer commandBuffer = pBuffer->GetNative();
		VkSubmitInfo submitInfo = {};
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;

		vkQueueSubmit(PVKInstance::GetQueue(m_Queue).queue, 1, &submitInfo, VK_NULL_HANDLE);
		vkQueueWaitIdle(PVKInstance::GetQueue(m_Queue).queue);

		vkFreeCommandBuffers(PVKInstance::GetDevice(), m_Pool, 1, &commandBuffer);
		RemoveCommandBuffer(pBuffer);
	}

	PVKCommandBuffer* PVKCommandPool::CreateCommandBuffer()
	{
		PVKCommandBuffer* pBuffer = new PVKCommandBuffer();
		pBuffer->Init(m_Pool);
		pBuffer->CreateCommandBuffer();
		m_Buffers.push_back(pBuffer);
		return pBuffer;
	}

	std::vector<PVKCommandBuffer*> PVKCommandPool::CreateCommandBuffers(uint32_t count)
	{
		std::vector<VkCommandBuffer> vkBuffers(count);

		VkCommandBufferAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		allocInfo.commandPool = m_Pool;
		allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		allocInfo.commandBufferCount = count;

		PVK_CHECK(vkAllocateCommandBuffers(PVKInstance::GetDevice(), &allocInfo, vkBuffers.data()), "Failed to allocate command buffers!")

		std::vector<PVKCommandBuffer*> b(count);
		for (size_t i = 0; i < count; i++) {
			b[i] = new PVKCommandBuffer;
			b[i]->Init(m_Pool);
			b[i]->SetCommandBuffer(vkBuffers[i]);
			m_Buffers.push_back(b[i]);
		}

		return b;
	}

	void PVKCommandPool::RemoveCommandBuffer(PVKCommandBuffer* pBuffer)
	{
		m_Buffers.erase(std::remove(m_Buffers.begin(), m_Buffers.end(), pBuffer), m_Buffers.end());
		delete pBuffer;
	}

	void PVKCommandPool::createCommandPool()
	{
		// Note: A command pool only supports one queue type at a time, for more queues more command pools are needed

		uint32_t queueIndex = PVKInstance::GetQueue(m_Queue).queueIndex;

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueIndex;
		poolInfo.flags = 0; // Optional

		PVK_CHECK(vkCreateCommandPool(PVKInstance::GetDevice(), &poolInfo, nullptr, &m_Pool), "Failed to create command pool!");
	}

}