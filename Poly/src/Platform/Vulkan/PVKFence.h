#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Platform/API/Fence.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKFence : public Fence
	{
	public:
		PVKFence() = default;
		~PVKFence();

		virtual void Init(FFenceFlag fenceFlag) override final;

		virtual void Reset() override final;

		virtual void Wait(uint64 timeout) override final;

		VkFence GetNativeVK() const { return m_Fence; }
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Fence); }
	
	private:
		VkFence m_Fence = VK_NULL_HANDLE;
	};
}
