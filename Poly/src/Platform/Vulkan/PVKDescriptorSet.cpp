#include "polypch.h"
#include "PVKDescriptorSet.h"

#include "PVKBuffer.h"
#include "PVKSampler.h"
#include "PVKInstance.h"
#include "PVKTextureView.h"
#include "PVKPipelineLayout.h"

namespace Poly
{

	PVKDescriptorSet::~PVKDescriptorSet()
	{
		// Pool also clears the set
		if (m_Pool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(PVKInstance::GetDevice(), m_Pool, nullptr);
	}

	void PVKDescriptorSet::Init(PipelineLayout* pLayout, uint32 setIndex)
	{
		m_SetIndex = setIndex;
		m_pPipelineLayout = reinterpret_cast<PVKPipelineLayout*>(pLayout);
		CreatePool(m_pPipelineLayout);
		CreateDescriptorSet(m_pPipelineLayout);
	}

	void PVKDescriptorSet::UpdateBufferBinding(uint32 binding, Buffer* pBuffer, uint64 offset, uint64 range)
	{
		// Note: This implementation is far from optimal in terms of performance.

		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer	= reinterpret_cast<PVKBuffer*>(pBuffer)->GetNativeVK();
		bufferInfo.offset	= offset;
		bufferInfo.range	= range;

		VkWriteDescriptorSet writeInfo = {};
		writeInfo.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfo.pNext				= nullptr;
		writeInfo.dstSet			= m_Set;
		writeInfo.dstBinding		= binding;
		writeInfo.descriptorCount	= 1;
		writeInfo.descriptorType	= ConvertDescriptorTypeVK(m_pPipelineLayout->GetBindings(m_SetIndex)[binding].DescriptorType);
		writeInfo.pBufferInfo		= &bufferInfo;
		writeInfo.pImageInfo		= nullptr;
		writeInfo.pTexelBufferView	= nullptr;
		writeInfo.dstArrayElement	= 0;

		vkUpdateDescriptorSets(PVKInstance::GetDevice(), 1, &writeInfo, 0, nullptr);
	}

	void PVKDescriptorSet::UpdateTextureBinding(uint32 binding, ETextureLayout layout, TextureView* pTextureView, Sampler* pSampler)
	{
		VkDescriptorImageInfo imageInfo = {};
		imageInfo.sampler		= reinterpret_cast<PVKSampler*>(pSampler)->GetNativeVK();
		imageInfo.imageView		= reinterpret_cast<PVKTextureView*>(pTextureView)->GetNativeVK();
		imageInfo.imageLayout	= ConvertTextureLayoutVK(layout);

		VkWriteDescriptorSet writeInfo = {};
		writeInfo.sType				= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		writeInfo.pNext				= nullptr;
		writeInfo.dstSet			= m_Set;
		writeInfo.dstBinding		= binding;
		writeInfo.descriptorCount	= 1;
		writeInfo.descriptorType	= ConvertDescriptorTypeVK(m_pPipelineLayout->GetBindings(m_SetIndex)[binding].DescriptorType);
		writeInfo.pBufferInfo		= nullptr;
		writeInfo.pImageInfo		= &imageInfo;
		writeInfo.pTexelBufferView	= nullptr;
		writeInfo.dstArrayElement	= 0;

		vkUpdateDescriptorSets(PVKInstance::GetDevice(), 1, &writeInfo, 0, nullptr);
	}

	void PVKDescriptorSet::CreatePool(PVKPipelineLayout* pLayout)
	{
		// Get pool sizes
		std::unordered_map<EDescriptorType, VkDescriptorPoolSize> pools;
		for (auto& bindings : pLayout->GetBindings(m_SetIndex)) {
			pools[bindings.DescriptorType].descriptorCount += 1;
			pools[bindings.DescriptorType].type = ConvertDescriptorTypeVK(bindings.DescriptorType);
		}

		// Store the new sorted data in a packed array
		std::vector<VkDescriptorPoolSize> poolSizes;
		poolSizes.reserve(pools.size());
		for (auto& pool : pools)
			poolSizes.push_back(pool.second);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount	= static_cast<uint32>(poolSizes.size());
		poolInfo.pPoolSizes		= poolSizes.data();
		poolInfo.maxSets		= 1; // Since this instance will only be for one set

		PVK_CHECK(vkCreateDescriptorPool(PVKInstance::GetDevice(), &poolInfo, nullptr, &m_Pool), "Failed to create descriptor pool with {} pools and {} max sets", poolInfo.poolSizeCount, poolInfo.maxSets);
	}

	void PVKDescriptorSet::CreateDescriptorSet(PVKPipelineLayout* pLayout)
	{
		m_SetLayout = pLayout->GetDescriptorSetLayoutVK(m_SetIndex);

		VkDescriptorSetAllocateInfo allocInfo = {};
		allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool		= m_Pool;
		allocInfo.descriptorSetCount	= 1;
		allocInfo.pSetLayouts			= &m_SetLayout;

		PVK_CHECK(vkAllocateDescriptorSets(PVKInstance::GetDevice(), &allocInfo, &m_Set), "Failed to allocate descriptor sets for set {}!", m_SetIndex);

	}

}