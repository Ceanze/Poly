#include "PVKPipelineLayout.h"

#include "polypch.h"
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
			std::vector<VkDescriptorBindingFlags>     bindingFlagsVK;
			bindingsVK.reserve(set.DescriptorSetBindings.size());
			bindingFlagsVK.reserve(set.DescriptorSetBindings.size());

			// Bindings in set
			// Bindings are saved for later use in Descriptors (Used when sets are updated in vkUpdateDescriptorSet)
			DescriptorSetLayout savedLayout = {};
			savedLayout.DescriptorSetBindings.reserve(set.DescriptorSetBindings.size());
			bool needsUpdateAfterBind = false;
			for (auto& binding : set.DescriptorSetBindings)
			{
				// Save custom type
				savedLayout.DescriptorSetBindings.push_back(binding);

				// Save VK type for creation
				VkDescriptorSetLayoutBinding bindingVK = {};
				bindingVK.binding                      = binding.Binding;
				bindingVK.descriptorCount              = binding.DescriptorCount;
				bindingVK.descriptorType               = ConvertDescriptorTypeVK(binding.DescriptorType);
				bindingVK.pImmutableSamplers           = nullptr; // TODO: Implement this
				bindingVK.stageFlags                   = ConvertShaderStageVK(binding.ShaderStage);
				bindingsVK.push_back(bindingVK);

				bindingFlagsVK.push_back(ConvertDescriptorBindingFlagVK(binding.BindingFlags));
				if (BitsSet(binding.BindingFlags, FDescriptorIndexingBindingFlag::UPDATE_AFTER_BIND))
					needsUpdateAfterBind = true;
			}
			m_DescriptorLayouts.push_back(savedLayout);

			// Descriptor indexing / bindless: per-binding flags (partially bound, variable count,
			// update-after-bind), only meaningfully populated for a bindless heap-style set
			VkDescriptorSetLayoutBindingFlagsCreateInfo bindingFlagsCreateInfo = {};
			bindingFlagsCreateInfo.sType                                       = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO;
			bindingFlagsCreateInfo.bindingCount                                = static_cast<uint32>(bindingFlagsVK.size());
			bindingFlagsCreateInfo.pBindingFlags                               = bindingFlagsVK.data();

			// Create VK Descriptor layout
			VkDescriptorSetLayout           vkLayout;
			VkDescriptorSetLayoutCreateInfo createInfo = {};
			createInfo.sType                           = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
			createInfo.bindingCount                    = static_cast<uint32>(bindingsVK.size());
			createInfo.pBindings                       = bindingsVK.data();
			createInfo.flags                           = needsUpdateAfterBind ? VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT : 0;
			createInfo.pNext                           = &bindingFlagsCreateInfo;
			vkCreateDescriptorSetLayout(PVKInstance::GetDevice(), &createInfo, nullptr, &vkLayout);
			m_DescriptorSetLayoutsVK.push_back(vkLayout);
		}
		m_DescriptorLayouts.shrink_to_fit();

		// Push constants
		for (auto& pushConstant : pDesc->PushConstantRanges)
		{
			VkPushConstantRange pushConstantVK = {};
			pushConstantVK.offset              = pushConstant.Offset;
			pushConstantVK.size                = pushConstant.Size;
			pushConstantVK.stageFlags          = ConvertShaderStageVK(pushConstant.ShaderStage);
			pushConstantRangesVK.push_back(pushConstantVK);
		}

		VkPipelineLayoutCreateInfo createInfo = {};
		createInfo.sType                      = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		createInfo.pNext                      = nullptr;
		createInfo.pPushConstantRanges        = pushConstantRangesVK.data();
		createInfo.pushConstantRangeCount     = static_cast<uint32>(pushConstantRangesVK.size());
		createInfo.pSetLayouts                = m_DescriptorSetLayoutsVK.data();
		createInfo.setLayoutCount             = static_cast<uint32>(m_DescriptorSetLayoutsVK.size());

		PVK_CHECK(vkCreatePipelineLayout(PVKInstance::GetDevice(), &createInfo, nullptr, &m_Layout), "Failed to create pipeline layout!");
	}
} // namespace Poly