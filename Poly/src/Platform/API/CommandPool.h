#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

namespace Poly
{
	class CommandBuffer;

	class CommandPool
	{
	public:
		CLASS_ABSTRACT(CommandPool);

		/**
		 * Init the CommandPool object
		 * @param queueType	Queue of which the CommandPool should be using and submit to
		*/
		virtual void Init(FQueueType queueType) = 0;

		/**
		 * Creates and allocates a command buffer belonging to the current pool
		 * Note: Destruction of the command buffer is handled by the command pool. Use FreeCommandBuffer to return it.
		 * @param commandBufferType	- Buffer type to create
		 * @return An allocated command buffer ready for use
		 */
		virtual CommandBuffer* AllocateCommandBuffer(ECommandBufferLevel commandBufferLevel) = 0;

		/**
		 * Free a previously allocated command buffer from the same command pool
		 * @param pCommandBuffer - Command buffer to free
		 */
		virtual void FreeCommandBuffer(CommandBuffer* pCommandBuffer) = 0;

		/**
		 * @return Native handle to the API specific object
		 */
		virtual uint64 GetNative() const = 0;

		/**
		 * @return Queue of the command pool
		 */
		inline FQueueType GetQueueType() const { return p_QueueType; }

	protected:
		FQueueType p_QueueType = FQueueType::NONE;
	};
}