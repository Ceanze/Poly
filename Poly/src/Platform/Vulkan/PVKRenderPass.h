#pragma once

#include "Platform/API/RenderPass.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKRenderPass : public RenderPass
	{
	public:
		PVKRenderPass() = default;
		~PVKRenderPass();

		virtual void Init(const RenderPassDesc* pDesc) override final;

		VkRenderPass GetNativeVK() const { return m_RenderPass; }
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_RenderPass); }

	private:
		VkRenderPass m_RenderPass = VK_NULL_HANDLE;
	};

}