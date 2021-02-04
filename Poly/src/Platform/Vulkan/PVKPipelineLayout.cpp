#include "polypch.h"
#include "PVKPipelineLayout.h"

#include "PVKInstance.h"

namespace Poly
{
	PVKPipelineLayout::~PVKPipelineLayout()
	{
		vkDestroyPipelineLayout(PVKInstance::GetDevice(), m_Layout, nullptr);
		m_Layout = VK_NULL_HANDLE;

		for (auto& descLayout : m_DescriptorSetLayoutsVK)
		{
			vkDestroyDescriptorSetLayout(PVKInstance::GetDevice(), descLayout, nullptr);
			descLayout = VK_NULL_HANDLE;
		}
	}

	void PVKPipelineLayout::Init(const PipelineLayoutDesc* pDesc)
	{
		p_PipelineLayoutDesc = *pDesc;

		m_DescriptorSetLayoutsVK.reserve(pDesc->DescriptorSetLayouts.size());
		std::vector<VkPushConstantRange> pushConstantRangesVK;
		pushConstantRangesVK.reserve(pDesc->PushConstantRanges.size());

		// Descriptor Sets
		m_DescriptorLayouts.reserve(pDesc->DescriptorSetLayouts.size());
		for (auto& set : pDesc->DescriptorSetLayouts)
		{
			std::vector<VkDescriptorSetLayoutBinding> bindingsVK;
			bindingsVK.reserve(set.DescriptorSetBindings.size());

			// Bindings in set
			// Bindings are saved for later use in Descriptors (Used when sets are updated in vkUpdateDescriptorSet)
			DescriptorSetLayout savedLayout = {};
			savedLayout.DescriptorSetBindings.reserve(set.DescriptorSetBindings.size());
			for (auto& binding : set.DescriptorSetBindings)
			{
				// Save custom type
				savedLayout.DescriptorSetBindings.push_back(binding);

				// Save VK type for creation
				VkDescriptorSetLayoutBinding bindingVK = {};
				bindingVK.binding				= binding.Binding;
				bindingVK.descriptorCount		= binding.DescriptorCount;
				bindingVK.descriptorType		= ConvertDescriptorTypeVK(binding.DescriptorType);
				bindingVK.pImmutableSamplers	= nullptr; // TODO: Implement this
				bindingVK.stageFlags			= ConvertShaderStageVK(binding.ShaderStage);
				bindingsVK.push_back(bindingVK);
			}
			m_DescriptorLayouts.push_back(savedLayout);

			// Create VK Descriptor layout
			VkDescriptorSetLayout vkLayout;
			VkDescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.sType		= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount	= bindingsVK.size();
			createInfo.pBindings	= bindingsVK.data();
			createInfo.flags		= 0;
			createInfo.pNext		= nullptr;
			vkCreateDescriptorSetLayout(PVKInstance::GetDevice(), &createInfo, nullptr, &vkLayout);
			m_DescriptorSetLayoutsVK.push_back(vkLayout);
		}
		m_DescriptorLayouts.shrink_to_fit();

		// Push constants
		for (auto& pushConstant : pDesc->PushConstantRanges)
		{
			VkPushConstantRange pushConstantVK = {};
			pushConstantVK.offset		= pushConstant.Offset;
			pushConstantVK.size			= pushConstant.Size;
			pushConstantVK.stageFlags	= ConvertShaderStageVK(pushConstant.ShaderStage);
			pushConstantRangesVK.push_back(pushConstantVK);
		}

		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext					= nullptr;
		createInfo.pPushConstantRanges		= pushConstantRangesVK.data();
		createInfo.pushConstantRangeCount	= pushConstantRangesVK.size();
		createInfo.pSetLayouts				= m_DescriptorSetLayoutsVK.data();
		createInfo.setLayoutCount			= m_DescriptorSetLayoutsVK.size();

		PVK_CHECK(vkCreatePipelineLayout(PVKInstance::GetDevice(), &createInfo, nullptr, &m_Layout), "Failed to create pipeline layout!");
	}
}