#pragma once

#include <vector>

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class Fence;
	class Semaphore;
	class CommandBuffer;

	class CommandQueue
	{
	public:
		CLASS_ABSTRACT(CommandQueue);

		/**
		 * Init the CommandQueue object
		 * @param queueType - Type of queue
		 * @param queueIndex - Index of queue to create from device
		*/
		virtual void Init(FQueueType queueType, uint32 queueIndex) = 0;

		/**
		 * Submits the command buffer and waits for queue to be finished
		 * Practially works like:
		 * 		SubmitQueueToGPU(...)
		 * 		WaitForDevice(...)
		 * @param commandBuffers - Vector of command buffer pointers to be submited
		 * @param pWaitSemaphore - (optional) Semaphore to wait on
		 */
		virtual void SubmitIdle(const std::vector<CommandBuffer*>& commandBuffers, Semaphore* pWaitSemaphore) = 0;

		/**
		 * Submit the command buffer with optional wait and signal semaphores
		 * @param commandBuffers	- Vector of command buffer points to be submited
		 * @param pWaitSemaphore	- Pointer to semaphore to wait for before submiting
		 * @param pSignalSemaphore	- Pointer to semaphore to signal when submission is complete
		 */
		virtual void Submit(const std::vector<CommandBuffer*>& commandBuffers, Semaphore* pWaitSemaphore, Semaphore* pSignalSemaphore, Fence* pFence) = 0;

		/**
		 * Waits for the queue to be idle
		 */
		virtual void Wait() = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;
	};
}