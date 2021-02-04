#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"
#include "Platform/API/Sampler.h"
#include "PVKTypes.h"

namespace Poly
{
	class PVKSampler : public Sampler
	{
	public:
		PVKSampler() = default;
		~PVKSampler();

		virtual void Init(const SamplerDesc* pDesc) override final;

		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Sampler); }
		VkSampler GetNativeVK() const { return m_Sampler; }

	private:
		VkSampler m_Sampler = VK_NULL_HANDLE;
	};
}