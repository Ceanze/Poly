#include "polypch.h"
#include "PVKCommandQueue.h"

#include "PVKFence.h"
#include "PVKInstance.h"
#include "PVKSemaphore.h"
#include "PVKCommandBuffer.h"

namespace Poly
{
	PVKCommandQueue::~PVKCommandQueue()
	{
		m_WaitSemaphores.clear();
	}

	void PVKCommandQueue::Init(FQueueType queueType, uint32 queueIndex)
	{
		// The command queues created by the device should already be created before this step by PVKInstance
		PVKQueue queue = PVKInstance::GetQueue(queueType, queueIndex);
		m_Queue = queue.queue;
		m_QueueIndex = queue.queueIndex;
		m_QueueType = queueType;
	}

	void PVKCommandQueue::SubmitIdle(const std::vector<CommandBuffer*>& commandBuffers, Semaphore* pWaitSemaphore)
	{
		Submit(commandBuffers, pWaitSemaphore, nullptr, nullptr);
		Wait();
	}

	void PVKCommandQueue::Submit(const std::vector<CommandBuffer*>& commandBuffers, Semaphore* pWaitSemaphore, Semaphore* pSignalSemaphore, Fence* pFence)
	{
		// Handle wait semaphores
		std::vector<VkSemaphore> waitSemaphores;
		std::vector<VkPipelineStageFlags> waitStages;
		if (!m_WaitSemaphores.empty() || pWaitSemaphore)
		{
			if (pWaitSemaphore) {
				waitSemaphores.push_back(reinterpret_cast<PVKSemaphore*>(pWaitSemaphore)->GetNativeVK());
				waitStages.push_back(reinterpret_cast<PVKSemaphore*>(pWaitSemaphore)->GetWaitStageMaskVK());
			}

			for (auto semaphore : m_WaitSemaphores)
			{
				waitSemaphores.push_back(semaphore->GetNativeVK());
				waitStages.push_back(semaphore->GetWaitStageMaskVK());
			}
		}

		// Handle signal semaphore
		VkSemaphore pSignalSemaphoreVK = VK_NULL_HANDLE;
		if (pSignalSemaphore)
			pSignalSemaphoreVK = reinterpret_cast<PVKSemaphore*>(pSignalSemaphore)->GetNativeVK();

		// Combine to VK command buffers
		std::vector<VkCommandBuffer> submitBuffers(commandBuffers.size());
		for (uint32 i = 0; i < commandBuffers.size(); i++)
			submitBuffers[i] = reinterpret_cast<PVKCommandBuffer*>(commandBuffers[i])->GetNativeVK();

		// Submit
		VkSubmitInfo submitInfo = {};
		submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pCommandBuffers		= submitBuffers.data();
		submitInfo.commandBufferCount	= static_cast<uint32>(submitBuffers.size());
		submitInfo.pWaitDstStageMask	= waitStages.data();
		submitInfo.waitSemaphoreCount	= static_cast<uint32>(waitSemaphores.size());
		submitInfo.pWaitSemaphores		= waitSemaphores.data();
		submitInfo.pNext				= nullptr;
		submitInfo.pSignalSemaphores	= &pSignalSemaphoreVK;
		submitInfo.signalSemaphoreCount	= pSignalSemaphore ? 1 : 0;

		PVK_CHECK(vkQueueSubmit(m_Queue, 1, &submitInfo, pFence ? reinterpret_cast<PVKFence*>(pFence)->GetNativeVK() : VK_NULL_HANDLE), "Failed to submit to queue with index {}", m_QueueIndex);

		if (!m_WaitSemaphores.empty()) m_WaitSemaphores.clear();
	}

	void PVKCommandQueue::AddWaitSemaphore(Semaphore* pWaitSemaphore)
	{
		m_WaitSemaphores.push_back(reinterpret_cast<PVKSemaphore*>(pWaitSemaphore));
	}

	void PVKCommandQueue::Wait()
	{
		PVK_CHECK(vkQueueWaitIdle(m_Queue), "Failed to wait for queue with index {}", m_QueueIndex);
	}
}