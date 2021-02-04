#include "polypch.h"
#include "PVKCommandPool.h"
#include "PVKInstance.h"
#include "VulkanCommon.h"
#include "PVKCommandBuffer.h"

namespace Poly
{
	PVKCommandPool::~PVKCommandPool()
	{
		vkDestroyCommandPool(PVKInstance::GetDevice(), m_Pool, nullptr);

		// Command buffers are automatically freed when command pool is destroyed
		for (auto buffer : m_Buffers) {
			delete buffer;
		}
		m_Buffers.clear();
	}

	void PVKCommandPool::Init(FQueueType queueType)
	{
		p_QueueType = queueType;
		CreateCommandPool();
	}

	CommandBuffer* PVKCommandPool::AllocateCommandBuffer(ECommandBufferLevel commandBufferLevel)
	{
		PVKCommandBuffer* pBuffer = new PVKCommandBuffer();
		pBuffer->Init(this);
		m_Buffers.push_back(pBuffer);
		return pBuffer;
	}

	void PVKCommandPool::FreeCommandBuffer(CommandBuffer* pCommandbuffer)
	{
		auto it = std::find(m_Buffers.begin(), m_Buffers.end(), reinterpret_cast<PVKCommandBuffer*>(pCommandbuffer));

		if (it != m_Buffers.end())
		{
			delete *it;
			m_Buffers.erase(it);
			VkCommandBuffer commandBuffer = reinterpret_cast<PVKCommandBuffer*>(pCommandbuffer)->GetNativeVK();
			vkFreeCommandBuffers(PVKInstance::GetDevice(), m_Pool, 1, &commandBuffer);
		}
		else
		{
			POLY_CORE_WARN("Tried to free a command buffer that was not part of the used pool!");
		}

	}

	void PVKCommandPool::CreateCommandPool()
	{
		uint32_t queueIndex = PVKInstance::GetQueue(p_QueueType).queueIndex;

		VkCommandPoolCreateInfo poolInfo = {};
		poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex	= queueIndex;
		poolInfo.flags				= 0;

		PVK_CHECK(vkCreateCommandPool(PVKInstance::GetDevice(), &poolInfo, nullptr, &m_Pool), "Failed to create command pool!");
	}

}