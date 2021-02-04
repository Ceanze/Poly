#pragma once

#include "Platform/API/Framebuffer.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKFramebuffer : public Framebuffer
	{
	public:
		PVKFramebuffer() = default;
		~PVKFramebuffer();

		virtual void Init(const FramebufferDesc* pDesc) override final;

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Framebuffer); }
		VkFramebuffer GetNativeVK() const { return m_Framebuffer; }

	private:
		VkFramebuffer m_Framebuffer = VK_NULL_HANDLE;
	};
}