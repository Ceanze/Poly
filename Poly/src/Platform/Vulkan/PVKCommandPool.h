#pragma once

#include <vector>

#include "PVKTypes.h"
#include "Platform/API/CommandPool.h"

namespace Poly
{
	class PVKCommandBuffer;

	class PVKCommandPool : public CommandPool
	{
	public:
		PVKCommandPool() = default;
		~PVKCommandPool();

		virtual void Init(FQueueType queueType) override final;

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Pool); }
		VkCommandPool GetNativeVK() const { return m_Pool; }

		virtual CommandBuffer* AllocateCommandBuffer(ECommandBufferLevel commandBufferLevel) override final;
		virtual void FreeCommandBuffer(CommandBuffer* pCommandbuffer) override final;

	private:
		void CreateCommandPool();

		VkCommandPool m_Pool = VK_NULL_HANDLE;
		std::vector<PVKCommandBuffer*> m_Buffers;
	};
}