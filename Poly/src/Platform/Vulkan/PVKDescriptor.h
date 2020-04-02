#pragma once

#include "polypch.h"
#include "PVKTypes.h"

#include <vulkan/vulkan.h>

namespace Poly
{

	class PVKBuffer;

	class PVKDescriptor
	{
	public:
		PVKDescriptor();
		~PVKDescriptor();

		void init(uint32_t copies);
		void cleanup();

		// Add binding to a set
		void addBinding(uint32_t set, uint32_t binding, BufferType bufferType, ShaderStage stageFlags);
		// Finish a set for creation
		void finalizeSet(uint32_t set);
		// Update the binding with the buffer for the whole range and no offset
		void updateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer& buffer);
		// Update the binding in the set for all copies
		void updateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer& buffer, VkDeviceSize offset, VkDeviceSize range);
		// Update the binding in the set for a specific copyIndex
		void updateBufferBinding(uint32_t copyIndex, uint32_t set, uint32_t binding, PVKBuffer& buffer, VkDeviceSize offset, VkDeviceSize range);
		// Get set layouts
		std::vector<VkDescriptorSetLayout> getSetLayouts();
		// Get set without any copies
		VkDescriptorSet getSet(uint32_t setIndex);
		// Get set from a copied descriptor set
		VkDescriptorSet getSet(uint32_t setIndex, uint32_t copyIndex);
		// Get all sets for a copy
		std::vector<VkDescriptorSet> getSets(uint32_t copyIndex);


	private:
		void createPool(uint32_t copies);
		void createDescriptorSets(uint32_t copies);

		VkDescriptorPool pool;
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>> setLayoutBindings;
		std::unordered_map<uint32_t, VkDescriptorSetLayout> setLayouts;
		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> descriptorSets;
	};

}