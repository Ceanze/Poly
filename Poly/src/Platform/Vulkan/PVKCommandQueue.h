#pragma once

#include <vector>

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Platform/API/CommandQueue.h"
#include "PVKTypes.h"

namespace Poly
{
	class Semaphore;
	class PVKSemaphore;
	class CommandBuffer;

	class PVKCommandQueue : public CommandQueue
	{
	public:
		PVKCommandQueue() = default;
		~PVKCommandQueue();

		virtual void Init(FQueueType queueType, uint32 queueIndex) override final;

		virtual void SubmitIdle(const std::vector<CommandBuffer*>& commandBuffers, Semaphore* pWaitSemaphore) override final;

		virtual void Submit(const std::vector<CommandBuffer*>& commandBuffers, Semaphore* pWaitSemaphore, Semaphore* pSignalSemaphore, Fence* pFence) override final;

		/**
		 * Adds a wait semaphore that the next submit to this queue will wait for
		 * @param pWaitSemaphore - Pointer to the semaphore to wait for
		 */
		void AddWaitSemaphore(Semaphore* pWaitSemaphore);

		virtual void Wait() override final;

		virtual uint64 GetNative() const { return reinterpret_cast<uint64>(m_Queue); }
		VkQueue GetNativeVK() const { return m_Queue;}
		uint32 GetQueueIndex() const { return m_QueueIndex; }

	private:
		std::vector<PVKSemaphore*> m_WaitSemaphores;
		uint32 m_QueueIndex		= 0;
		VkQueue m_Queue			= VK_NULL_HANDLE;
		FQueueType m_QueueType	= FQueueType::NONE;
	};
}