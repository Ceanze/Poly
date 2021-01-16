#include "polypch.h"
#include "PVKDescriptor.h"

#include "PVKInstance.h"
#include "PVKBuffer.h"
#include "PVKSampler.h"
#include "PVKTexture.h"

namespace Poly
{

	PVKDescriptor::PVKDescriptor()
	{
	}

	PVKDescriptor::~PVKDescriptor()
	{
	}

	void PVKDescriptor::Init(uint32_t copies)
	{
		CreatePool(copies);
		CreateDescriptorSets(copies);
	}

	void PVKDescriptor::Cleanup()
	{
		if (m_Pool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(PVKInstance::GetDevice(), m_Pool, nullptr);

		for (auto& layout : m_SetLayouts)
			vkDestroyDescriptorSetLayout(PVKInstance::GetDevice(), layout.second, nullptr);
	}

	void PVKDescriptor::AddBinding(uint32_t set, uint32_t binding, BufferType bufferType, FShaderStage stageFlags)
	{
		VkDescriptorSetLayoutBinding layout = {};
		layout.binding = binding;
		layout.descriptorCount = 1;
		layout.descriptorType = (VkDescriptorType)bufferType;
		layout.pImmutableSamplers = nullptr; // Might be added in future for samplers/textures
		layout.stageFlags = ConvertShaderStageVK(stageFlags);
		
		m_SetLayoutBindings[set][binding] = layout;
	}

	void PVKDescriptor::FinalizeSet(uint32_t set)
	{
		auto it = m_SetLayoutBindings.find(set);
		if (it == m_SetLayoutBindings.end()) {
			POLY_CORE_ERROR("Could not finalize set {} as it has no bindings added!", set);
			return;
		}

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto& set : m_SetLayoutBindings)
			for (auto& layout : set.second)
				bindings.push_back(layout.second);

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = m_SetLayoutBindings[set].size();
		info.pBindings = bindings.data();

		PVK_CHECK(vkCreateDescriptorSetLayout(PVKInstance::GetDevice(), &info, nullptr, &m_SetLayouts[set]), "Failed to create descriptor set {}!", set);
	}

	void PVKDescriptor::UpdateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer* pBuffer)
	{
		UpdateBufferBinding(set, binding, pBuffer, 0, pBuffer->GetSize());
	}

	void PVKDescriptor::UpdateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer* pBuffer, VkDeviceSize offset, VkDeviceSize range)
	{
		// To optimise the vkUpdateDescriptorSets should only be called once instead for each binding

		std::vector<VkWriteDescriptorSet> writeSets;
		uint32_t numCopies = m_DescriptorSets[set].size();
		for (uint32_t i = 0; i < numCopies; i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = pBuffer->GetNativeVK();
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[set][i];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = m_SetLayoutBindings[set][binding].descriptorType;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			writeSets.push_back(descriptorWrite);

		}
		vkUpdateDescriptorSets(PVKInstance::GetDevice(), writeSets.size(), writeSets.data(), 0, nullptr);
	}

	void PVKDescriptor::UpdateBufferBinding(uint32_t copyIndex, uint32_t set, uint32_t binding, PVKBuffer* pBuffer, VkDeviceSize offset, VkDeviceSize range)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = pBuffer->GetNativeVK();
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = m_DescriptorSets[set][copyIndex];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = m_SetLayoutBindings[set][binding].descriptorType;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(PVKInstance::GetDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	void PVKDescriptor::UpdateTextureBinding(uint32_t set, uint32_t binding, ImageLayout layout, PVKTexture* pTexture, PVKSampler* pSampler)
	{
		std::vector<VkWriteDescriptorSet> writeSets;
		uint32_t numCopies = m_DescriptorSets[set].size();
		for (uint32_t i = 0; i < numCopies; i++) {
			VkDescriptorImageInfo imageInfo = {};
			imageInfo.imageLayout = (VkImageLayout)layout;
			imageInfo.imageView = pTexture->GetImageViewVK();
			imageInfo.sampler = pSampler->GetNative();


			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = m_DescriptorSets[set][i];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = m_SetLayoutBindings[set][binding].descriptorType;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = nullptr;
			descriptorWrite.pImageInfo = &imageInfo;
			descriptorWrite.pTexelBufferView = nullptr;

			writeSets.push_back(descriptorWrite);

		}
		// Crash here is probably because shader isnt updated
		vkUpdateDescriptorSets(PVKInstance::GetDevice(), writeSets.size(), writeSets.data(), 0, nullptr);
	}

	std::vector<VkDescriptorSetLayout> PVKDescriptor::GetSetLayouts()
	{
		std::vector<VkDescriptorSetLayout> v;
		for (auto& layout : m_SetLayouts)
			v.push_back(layout.second);
		return v;
	}

	VkDescriptorSet PVKDescriptor::GetSet(uint32_t setIndex)
	{
		return m_DescriptorSets[setIndex][0];
	}

	VkDescriptorSet PVKDescriptor::GetSet(uint32_t setIndex, uint32_t copyIndex)
	{
		return m_DescriptorSets[setIndex][copyIndex];
	}

	std::vector<VkDescriptorSet> PVKDescriptor::GetSets(uint32_t copyIndex)
	{
		std::vector<VkDescriptorSet> sets;
		for (auto& set : m_DescriptorSets) {
			sets.push_back(set.second[copyIndex]);
		}
		return sets;
	}

	void PVKDescriptor::CreatePool(uint32_t copies)
	{
		// Get pool sizes
		uint32_t setCount = 0;
		std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> pools;
		for (auto& layoutBinding : m_SetLayoutBindings) {
			for (auto& bindings : layoutBinding.second) {
				pools[bindings.second.descriptorType].descriptorCount += 1;
			}
			setCount += 1;
		}

		// Set their descriptor types and multiply by the copies amount
		for (auto& type : pools) {
			type.second.type = type.first;
			type.second.descriptorCount *= copies;
		}

		std::vector<VkDescriptorPoolSize> poolSizes;
		for (auto& pool : pools)
			poolSizes.push_back(pool.second);

		VkDescriptorPoolCreateInfo poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = poolSizes.size();
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = setCount * copies;

		PVK_CHECK(vkCreateDescriptorPool(PVKInstance::GetDevice(), &poolInfo, nullptr, &m_Pool), "Failed to create descriptor pool with {} pools and {} max sets", poolInfo.poolSizeCount, poolInfo.maxSets);
	}

	void PVKDescriptor::CreateDescriptorSets(uint32_t copies)
	{
		// Creates the descriptor sets with their bindings, does not know about any data, which is added with an update

		for (auto& set : m_SetLayouts) {
			std::vector<VkDescriptorSetLayout> layouts(copies, set.second);
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = m_Pool;
			allocInfo.descriptorSetCount = copies;
			allocInfo.pSetLayouts = layouts.data();

			m_DescriptorSets[set.first].resize(copies);
			PVK_CHECK(vkAllocateDescriptorSets(PVKInstance::GetDevice(), &allocInfo, m_DescriptorSets[set.first].data()), "Failed to allocate descriptor sets for set {}!", set.first);
		}
	}

}