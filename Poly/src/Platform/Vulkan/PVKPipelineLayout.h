#pragma once

#include <vector>

#include "Poly/Core/Core.h"
#include "PVKTypes.h"
#include "Platform/API/PipelineLayout.h"

namespace Poly
{
	class PVKPipelineLayout : public PipelineLayout
	{
	public:
		PVKPipelineLayout() = default;
		~PVKPipelineLayout();

		virtual void Init(const PipelineLayoutDesc* pDesc) override final;

		const std::vector<DescriptorSetBinding>& GetBindings(uint32 setIndex) const { m_DescriptorLayouts[setIndex].DescriptorSetBindings; }
		VkDescriptorSetLayout GetDescriptorSetLayoutVK(uint32 setIndex) const { m_DescriptorSetLayoutsVK[setIndex]; }
		const std::vector<VkDescriptorSetLayout>& GetDescriptorSetLayoutsVK() const { return m_DescriptorSetLayoutsVK; }

		VkPipelineLayout GetNativeVK() const { return m_Layout; }
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Layout); };

	private:
		VkPipelineLayout					m_Layout = VK_NULL_HANDLE;
		std::vector<DescriptorSetLayout>	m_DescriptorLayouts;
		std::vector<VkDescriptorSetLayout>	m_DescriptorSetLayoutsVK;
	};
}