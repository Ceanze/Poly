#pragma once

#include "polypch.h"

#include <vulkan/vulkan.h>

namespace Poly
{

	class PVKDescriptor
	{
	public:
		enum class Type
		{
			SAMPLER = 0,
			COMBINED_IMAGE_SAMPLER = 1,
			SAMPLED_IMAGE = 2,
			STORAGE_IMAGE = 3,
			UNIFORM_TEXEL_BUFFER = 4,
			STORAGE_TEXEL_BUFFER = 5,
			UNIFORM_BUFFER = 6,
			STORAGE_BUFFER = 7,
			UNIFORM_BUFFER_DYNAMIC = 8,
			STORAGE_BUFFER_DYNAMIC = 9,
			INPUT_ATTACHMENT = 10,
		};

	public:
		PVKDescriptor();
		~PVKDescriptor();

		void init(uint32_t copies);
		void cleanup();

		// Add binding to a set
		void addBinding(uint32_t set, uint32_t binding, Type bufferType, VkShaderStageFlags stageFlags);
		// Finish a set for creation
		void finalizeSet(uint32_t set);
		// TODO: CHANGE VkBuffer TO PVKBuffer WHEN THAT IS POSSIBLE
		void updateBufferBinding(uint32_t set, uint32_t binding, VkBuffer buffer);
		// Update the binding in the set for all copies
		void updateBufferBinding(uint32_t set, uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
		// Update the binding in the set for a specific copyIndex
		void updateBufferBinding(uint32_t copyIndex, uint32_t set, uint32_t binding, VkBuffer buffer, VkDeviceSize offset, VkDeviceSize range);
		// Get set layouts
		std::vector<VkDescriptorSetLayout> getSetLayouts();
		// Get set without any copies
		VkDescriptorSet getSet(uint32_t setIndex);
		// Get set from a copied descriptor set
		VkDescriptorSet getSet(uint32_t setIndex, uint32_t copyIndex);


	private:
		void createPool(uint32_t copies);
		void createDescriptorSets(uint32_t copies);

		VkDescriptorPool pool;
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>> setLayoutBindings;
		std::unordered_map<uint32_t, VkDescriptorSetLayout> setLayouts;
		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> descriptorSets;
	};

}