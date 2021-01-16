#pragma once

#include "polypch.h"
#include "PVKTypes.h"

#include <vulkan/vulkan.h>

namespace Poly
{

	class PVKBuffer;
	class PVKTexture;
	class PVKSampler;

	class PVKDescriptor
	{
	public:
		PVKDescriptor();
		~PVKDescriptor();

		void Init(uint32_t copies);
		void Cleanup();

		// Add binding to a set
		void AddBinding(uint32_t set, uint32_t binding, BufferType bufferType, FShaderStage stageFlags);
		// Finish a set for creation
		void FinalizeSet(uint32_t set);

		// Update the binding with the buffer for the whole range and no offset
		void UpdateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer* pBuffer);
		// Update the binding in the set for all copies
		void UpdateBufferBinding(uint32_t set, uint32_t binding, PVKBuffer* pBuffer, VkDeviceSize offset, VkDeviceSize range);
		// Update the binding in the set for a specific copyIndex
		void UpdateBufferBinding(uint32_t copyIndex, uint32_t set, uint32_t binding, PVKBuffer* pBuffer, VkDeviceSize offset, VkDeviceSize range);
		// Update the image binding
		void UpdateTextureBinding(uint32_t set, uint32_t binding, ImageLayout layout, PVKTexture* pTexture, PVKSampler* pSampler);

		// Get set layouts
		std::vector<VkDescriptorSetLayout> GetSetLayouts();
		// Get set without any copies
		VkDescriptorSet GetSet(uint32_t setIndex);
		// Get set from a copied descriptor set
		VkDescriptorSet GetSet(uint32_t setIndex, uint32_t copyIndex);
		// Get all sets for a copy
		std::vector<VkDescriptorSet> GetSets(uint32_t copyIndex);


	private:
		void CreatePool(uint32_t copies);
		void CreateDescriptorSets(uint32_t copies);

		VkDescriptorPool m_Pool = VK_NULL_HANDLE;
		std::unordered_map<uint32_t, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding>> m_SetLayoutBindings;
		std::unordered_map<uint32_t, VkDescriptorSetLayout> m_SetLayouts;
		std::unordered_map<uint32_t, std::vector<VkDescriptorSet>> m_DescriptorSets;
	};

}