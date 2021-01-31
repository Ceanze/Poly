#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Platform/API/Semaphore.h"

namespace Poly
{
	class PVKSemaphore : public Semaphore
	{
	public:
		PVKSemaphore() = default;
		~PVKSemaphore();

		virtual void Init() override final;

		virtual void AddWaitStageMask(FPipelineStage stage) override final;

		virtual FPipelineStage GetWaitStageMask() const override final { return m_WaitStage; }
		VkPipelineStageFlags GetWaitStageMaskVK() const { return ConvertPipelineStageFlagsVK(m_WaitStage); }

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Semaphore); };

		VkSemaphore GetNativeVK() const { return m_Semaphore; }

	private:
		VkSemaphore m_Semaphore		= VK_NULL_HANDLE;
		FPipelineStage m_WaitStage	= FPipelineStage::NONE;
	};
}