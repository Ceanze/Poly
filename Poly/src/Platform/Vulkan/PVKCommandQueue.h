#pragma once

#include "Platform/API/CommandQueue.h"
#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "PVKTypes.h"

namespace Poly
{
	class BinarySemaphore;
	class PVKBinarySemaphore;
	class CommandBuffer;

	class PVKCommandQueue : public CommandQueue
	{
	public:
		PVKCommandQueue()  = default;
		~PVKCommandQueue() = default;

		virtual void Init(FQueueType queueType, uint32 queueIndex) override final;

		virtual void SubmitIdle(const SubmitDesc& submitDesc) override final;

		virtual void Submit(const SubmitDesc& submitDesc) override final;

		virtual void Wait() override final;

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Queue.queue); }
		VkQueue        GetNativeVK() const { return m_Queue.queue; }
		virtual uint32 GetQueueIndex() const override final { return m_Queue.queueIndex; }
		virtual uint32 GetQueueFamilyIndex() const override final { return m_Queue.queueFamilyIndex; }

	private:
		std::string GetQueueName();

		PVKQueue   m_Queue;
		FQueueType m_QueueType = FQueueType::NONE;
	};
} // namespace Poly