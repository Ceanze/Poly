#pragma once

#include "Platform/API/SyncPoint.h"
#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKSyncPoint : public SyncPoint
	{
	public:
		PVKSyncPoint() = default;
		~PVKSyncPoint();

		virtual void Init() override final;

		virtual void   Wait(uint64 value) override final;
		virtual void   Signal(uint64 value) override final;
		virtual uint64 GetValue() const override final;

		virtual void AddWaitStageMask(FPipelineStage stage) override final;

		virtual FPipelineStage GetWaitStageMask() const override final { return m_WaitStage; }
		VkPipelineStageFlags   GetWaitStageMaskVK() const { return ConvertPipelineStageFlagsVK(m_WaitStage); }
		virtual void           ClearWaitStageMask() override final { m_WaitStage = FPipelineStage::NONE; }

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_TimelineSemaphore); }

		VkSemaphore GetNativeVK() const { return m_TimelineSemaphore; }

	private:
		VkSemaphore    m_TimelineSemaphore = VK_NULL_HANDLE;
		FPipelineStage m_WaitStage         = FPipelineStage::NONE;
	};
} // namespace Poly