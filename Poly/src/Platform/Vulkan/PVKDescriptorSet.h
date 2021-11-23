#pragma once

#include "polypch.h"
#include "PVKTypes.h"
#include "Platform/API/DescriptorSet.h"
#include "Platform/Vulkan/PVKPipelineLayout.h"

namespace Poly
{
	class PVKBuffer;
	class PVKSampler;
	//class PVKPipelineLayout;

	class PVKDescriptorSet : public DescriptorSet
	{
	public:
		PVKDescriptorSet() = default;
		~PVKDescriptorSet();

		virtual void Init(PipelineLayout* pLayout, uint32 setIndex) override final;

		virtual void UpdateBufferBinding(uint32 binding, const Buffer* pBuffer, uint64 offset, uint64 range) override final;
		virtual void UpdateTextureBinding(uint32 binding, ETextureLayout layout, const TextureView* pTextureView, Sampler* pSampler) override final;

		VkDescriptorSetLayout GetSetLayout() const { return m_SetLayout; };
		virtual uint64 GetNative() const override final { return reinterpret_cast<uint64>(m_Set); }
		VkDescriptorSet GetNativeVK() const { return m_Set; }
		virtual PipelineLayout* GetLayout() const override final { return m_pPipelineLayout; };
		virtual uint32 GetSetIndex() const override final { return m_SetIndex; };

	private:
		void CreatePool(PVKPipelineLayout* pLayout);
		void CreateDescriptorSet(PVKPipelineLayout* pLayout);

		VkDescriptorSet			m_Set				= VK_NULL_HANDLE;
		VkDescriptorPool		m_Pool				= VK_NULL_HANDLE;
		VkDescriptorSetLayout	m_SetLayout			= VK_NULL_HANDLE;
		uint32					m_SetIndex			= 0;
		PVKPipelineLayout*		m_pPipelineLayout	= nullptr;
	};
}