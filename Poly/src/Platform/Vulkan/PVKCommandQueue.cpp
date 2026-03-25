#include "PVKCommandQueue.h"

#include "polypch.h"
#include "PVKBinarySemaphore.h"
#include "PVKCommandBuffer.h"
#include "PVKInstance.h"
#include "PVKSyncPoint.h"

namespace Poly
{
	void PVKCommandQueue::Init(FQueueType queueType, uint32 queueIndex)
	{
		// The command queues created by the device should already be created before this step by PVKInstance
		m_Queue     = PVKInstance::GetQueue(queueType, queueIndex);
		m_QueueType = queueType;
	}

	void PVKCommandQueue::SubmitIdle(const SubmitDesc& submitDesc)
	{
		Submit(submitDesc);
		Wait();
	}

	void PVKCommandQueue::Submit(const SubmitDesc& submitDesc)
	{
		// Command Buffers
		std::vector<VkCommandBufferSubmitInfo> commandBufferInfos;
		commandBufferInfos.reserve(submitDesc.CommandBuffers.size());
		for (const auto& pCommandBuffer : submitDesc.CommandBuffers)
		{
			VkCommandBufferSubmitInfo commandBufferInfo = {};
			commandBufferInfo.sType                     = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandBufferInfo.commandBuffer             = reinterpret_cast<PVKCommandBuffer*>(pCommandBuffer)->GetNativeVK();
			commandBufferInfo.pNext                     = nullptr;
			commandBufferInfo.deviceMask                = 0;
			commandBufferInfos.push_back(commandBufferInfo);
		}

		// Wait binary/timeline semaphores
		std::vector<VkSemaphoreSubmitInfo> waitSemaphoreInfos;
		waitSemaphoreInfos.reserve(submitDesc.WaitSemaphores.size() + submitDesc.WaitSyncPoints.size());
		for (const auto& pWaitSemaphore : submitDesc.WaitSemaphores)
		{
			VkSemaphoreSubmitInfo semaphoreInfo = {};
			semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			semaphoreInfo.semaphore             = reinterpret_cast<PVKBinarySemaphore*>(pWaitSemaphore)->GetNativeVK();
			semaphoreInfo.stageMask             = reinterpret_cast<PVKBinarySemaphore*>(pWaitSemaphore)->GetWaitStageMaskVK();
			semaphoreInfo.pNext                 = nullptr;
			semaphoreInfo.deviceIndex           = 0;
			waitSemaphoreInfos.push_back(semaphoreInfo);
		}

		for (const auto& waitSemaphore : submitDesc.WaitSyncPoints)
		{
			VkSemaphoreSubmitInfo semaphoreInfo = {};
			semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			semaphoreInfo.semaphore             = reinterpret_cast<PVKSyncPoint*>(waitSemaphore.pSyncPoint)->GetNativeVK();
			semaphoreInfo.stageMask             = reinterpret_cast<PVKSyncPoint*>(waitSemaphore.pSyncPoint)->GetWaitStageMaskVK();
			semaphoreInfo.pNext                 = nullptr;
			semaphoreInfo.deviceIndex           = 0;
			semaphoreInfo.value                 = waitSemaphore.Value;
			waitSemaphoreInfos.push_back(semaphoreInfo);
		}

		// Signal binary/timeline semaphores
		std::vector<VkSemaphoreSubmitInfo> signalSemaphoreInfos;
		signalSemaphoreInfos.reserve(submitDesc.SignalSemaphores.size() + submitDesc.SignalSyncPoints.size());
		for (const auto& pSignalSemaphore : submitDesc.SignalSemaphores)
		{
			VkSemaphoreSubmitInfo semaphoreInfo = {};
			semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			semaphoreInfo.semaphore             = reinterpret_cast<PVKBinarySemaphore*>(pSignalSemaphore)->GetNativeVK();
			semaphoreInfo.stageMask             = reinterpret_cast<PVKBinarySemaphore*>(pSignalSemaphore)->GetWaitStageMaskVK();
			semaphoreInfo.pNext                 = nullptr;
			semaphoreInfo.deviceIndex           = 0;
			signalSemaphoreInfos.push_back(semaphoreInfo);
		}

		for (const auto& signalSemaphore : submitDesc.SignalSyncPoints)
		{
			VkSemaphoreSubmitInfo semaphoreInfo = {};
			semaphoreInfo.sType                 = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
			semaphoreInfo.semaphore             = reinterpret_cast<PVKSyncPoint*>(signalSemaphore.pSyncPoint)->GetNativeVK();
			semaphoreInfo.stageMask             = reinterpret_cast<PVKSyncPoint*>(signalSemaphore.pSyncPoint)->GetWaitStageMaskVK();
			semaphoreInfo.pNext                 = nullptr;
			semaphoreInfo.deviceIndex           = 0;
			semaphoreInfo.value                 = signalSemaphore.Value;
			signalSemaphoreInfos.push_back(semaphoreInfo);
		}

		// Submit
		VkSubmitInfo2 submitInfo            = {};
		submitInfo.sType                    = VK_STRUCTURE_TYPE_SUBMIT_INFO_2;
		submitInfo.pCommandBufferInfos      = commandBufferInfos.data();
		submitInfo.commandBufferInfoCount   = commandBufferInfos.size();
		submitInfo.pSignalSemaphoreInfos    = signalSemaphoreInfos.data();
		submitInfo.signalSemaphoreInfoCount = signalSemaphoreInfos.size();
		submitInfo.pWaitSemaphoreInfos      = waitSemaphoreInfos.data();
		submitInfo.waitSemaphoreInfoCount   = waitSemaphoreInfos.size();
		submitInfo.pNext                    = nullptr;

		PVK_CHECK(vkQueueSubmit2(m_Queue.queue, 1, &submitInfo, VK_NULL_HANDLE), "Failed to submit to {} queue with index {}", GetQueueName(), m_Queue.queueIndex);
	}

	void PVKCommandQueue::Wait()
	{
		PVK_CHECK(vkQueueWaitIdle(m_Queue.queue), "Failed to wait for {} queue with index {}", GetQueueName(), m_Queue.queueIndex);
	}

	std::string PVKCommandQueue::GetQueueName()
	{
		switch (m_QueueType)
		{
		case FQueueType::GRAPHICS:
			return "GRAPHICS";
		case FQueueType::COMPUTE:
			return "COMPUTE";
		case FQueueType::TRANSFER:
			return "TRANSFER";
		case FQueueType::SPARSE_BINDING:
			return "SPARSE_BINDING";
		case FQueueType::VIDEO_DECODE:
			return "VIDEO_DECODE";
		case FQueueType::VIDEO_ENCODE:
			return "VIDEO_ENCODE";
		case FQueueType::OPTICAL_FLOW:
			return "OPTICAL_FLOW";
		case FQueueType::DATA_GRAPH:
			return "DATA_GRAPH";
		default:
			return "[No queue type found]";
		}
	}
} // namespace Poly