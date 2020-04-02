#include "polypch.h"
#include "PVKDescriptor.h"

#include "PVKInstance.h"
#include "PVKBuffer.h"

namespace Poly
{

	PVKDescriptor::PVKDescriptor() : pool(VK_NULL_HANDLE)
	{
	}

	PVKDescriptor::~PVKDescriptor()
	{
	}

	void PVKDescriptor::init(uint32_t copies)
	{
		createPool(copies);
		createDescriptorSets(copies);
	}

	void PVKDescriptor::cleanup()
	{
		if (pool != VK_NULL_HANDLE)
			vkDestroyDescriptorPool(PVKInstance::getDevice(), this->pool, nullptr);

		for (auto& layout : this->setLayouts)
			vkDestroyDescriptorSetLayout(PVKInstance::getDevice(), layout.second, nullptr);
	}

	void PVKDescriptor::addBinding(uint32_t set, uint32_t binding, BufferType bufferType, ShaderStage stageFlags)
	{
		VkDescriptorSetLayoutBinding layout = {};
		layout.binding = binding;
		layout.descriptorCount = 1;
		layout.descriptorType = (VkDescriptorType)bufferType;
		layout.pImmutableSamplers = nullptr; // Might be added in future for samplers/textures
		layout.stageFlags = (VkShaderStageFlagBits)stageFlags;
		
		this->setLayoutBindings[set][binding] = layout;
	}

	void PVKDescriptor::finalizeSet(uint32_t set)
	{
		auto it = this->setLayoutBindings.find(set);
		if (it == this->setLayoutBindings.end()) {
			POLY_CORE_ERROR("Could not finalize set {} as it has no bindings added!", set);
			return;
		}

		std::vector<VkDescriptorSetLayoutBinding> bindings;
		for (auto& set : this->setLayoutBindings)
			for (auto& layout : set.second)
				bindings.push_back(layout.second);

		VkDescriptorSetLayoutCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		info.bindingCount = this->setLayoutBindings[set].size();
		info.pBindings = bindings.data();

		PVK_CHECK(vkCreateDescriptorSetLayout(PVKInstance::getDevice(), &info, nullptr, &setLayouts[set]), "Failed to create descriptor set {}!", set);
	}

	void PVKDescriptor::updateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer& buffer)
	{
		updateBufferBinding(set, binding, buffer, 0, buffer.getSize());
	}

	void PVKDescriptor::updateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer& buffer, VkDeviceSize offset, VkDeviceSize range)
	{
		// To optimise the vkUpdateDescriptorSets should only be called once instead for each binding

		std::vector<VkWriteDescriptorSet> writeSets;
		uint32_t numCopies = this->descriptorSets[set].size();
		for (uint32_t i = 0; i < numCopies; i++) {
			VkDescriptorBufferInfo bufferInfo = {};
			bufferInfo.buffer = buffer.getNative();
			bufferInfo.offset = offset;
			bufferInfo.range = range;

			VkWriteDescriptorSet descriptorWrite = {};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = this->descriptorSets[set][i];
			descriptorWrite.dstBinding = binding;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = this->setLayoutBindings[set][binding].descriptorType;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrite.pImageInfo = nullptr;
			descriptorWrite.pTexelBufferView = nullptr;

			writeSets.push_back(descriptorWrite);

		}
		vkUpdateDescriptorSets(PVKInstance::getDevice(), writeSets.size(), writeSets.data(), 0, nullptr);
	}

	void PVKDescriptor::updateBufferBinding(uint32_t copyIndex, uint32_t set, uint32_t binding, PVKBuffer& buffer, VkDeviceSize offset, VkDeviceSize range)
	{
		VkDescriptorBufferInfo bufferInfo = {};
		bufferInfo.buffer = buffer.getNative();
		bufferInfo.offset = offset;
		bufferInfo.range = range;

		VkWriteDescriptorSet descriptorWrite = {};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = this->descriptorSets[set][copyIndex];
		descriptorWrite.dstBinding = binding;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = this->setLayoutBindings[set][binding].descriptorType;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;

		vkUpdateDescriptorSets(PVKInstance::getDevice(), 1, &descriptorWrite, 0, nullptr);
	}

	std::vector<VkDescriptorSetLayout> PVKDescriptor::getSetLayouts()
	{
		std::vector<VkDescriptorSetLayout> v;
		for (auto& layout : this->setLayouts)
			v.push_back(layout.second);
		return v;
	}

	VkDescriptorSet PVKDescriptor::getSet(uint32_t setIndex)
	{
		return this->descriptorSets[setIndex][0];
	}

	VkDescriptorSet PVKDescriptor::getSet(uint32_t setIndex, uint32_t copyIndex)
	{
		return this->descriptorSets[setIndex][copyIndex];
	}

	std::vector<VkDescriptorSet> PVKDescriptor::getSets(uint32_t copyIndex)
	{
		std::vector<VkDescriptorSet> sets;
		for (auto& set : this->descriptorSets) {
			sets.push_back(set.second[copyIndex]);
		}
		return sets;
	}

	void PVKDescriptor::createPool(uint32_t copies)
	{
		// Get pool sizes
		uint32_t setCount = 0;
		std::unordered_map<VkDescriptorType, VkDescriptorPoolSize> pools;
		for (auto& layoutBinding : this->setLayoutBindings) {
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

		PVK_CHECK(vkCreateDescriptorPool(PVKInstance::getDevice(), &poolInfo, nullptr, &this->pool), "Failed to create descriptor pool with {} pools and {} max sets", poolInfo.poolSizeCount, poolInfo.maxSets);
	}

	void PVKDescriptor::createDescriptorSets(uint32_t copies)
	{
		// Creates the descriptor sets with their bindings, does not know about any data, which is added with an update

		for (auto& set : this->setLayouts) {
			std::vector<VkDescriptorSetLayout> layouts(copies, set.second);
			VkDescriptorSetAllocateInfo allocInfo = {};
			allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
			allocInfo.descriptorPool = this->pool;
			allocInfo.descriptorSetCount = copies;
			allocInfo.pSetLayouts = layouts.data();

			this->descriptorSets[set.first].resize(copies);
			PVK_CHECK(vkAllocateDescriptorSets(PVKInstance::getDevice(), &allocInfo, this->descriptorSets[set.first].data()), "Failed to allocate descriptor sets for set {}!", set.first);
		}
	}

}