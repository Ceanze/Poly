#pragma once

#include "PVKTypes.h"

#include <vector>

namespace Poly
{
	class PVKCommandBuffer;

	class PVKCommandPool
	{
	public:
		PVKCommandPool();
		~PVKCommandPool();

		void Init(QueueType queueType);
		void Cleanup();

		VkCommandPool GetNative() const { return m_Pool; }
		QueueType GetQueueType() const;

		PVKCommandBuffer* BeginSingleTimeCommand();
		void EndSingleTimeCommand(PVKCommandBuffer* pBuffer);

		PVKCommandBuffer* CreateCommandBuffer();
		std::vector<PVKCommandBuffer*> CreateCommandBuffers(uint32_t count);
		void RemoveCommandBuffer(PVKCommandBuffer* pBuffer);

	private:
		void createCommandPool();

		VkCommandPool	m_Pool	= VK_NULL_HANDLE;
		QueueType		m_Queue	= QueueType::GRAPHICS;

		std::vector<PVKCommandBuffer*> m_Buffers;
	};
}