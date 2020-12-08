#pragma once

#include "polypch.h"
#include "PVKTypes.h"
#include "Platform/API/Buffer.h"

#include "VmaInclude.h"

namespace Poly
{
	class PVKBuffer : public Buffer
	{
	public:
		PVKBuffer();
		~PVKBuffer();

		// TODO: Fix init to only have the API one
		void Init(VkDeviceSize size, BufferUsage usage, const std::vector<uint32_t>& queueFamilyIndices, VmaMemoryUsage memoryUsage);
		virtual void Init(const BufferDesc* pDesc) override final;
		void Cleanup();

		virtual void* Map() override final;
		virtual void TransferData(const void* data, const size_t size) override final;
		virtual void Unmap() override final;

		virtual uint64_t GetSize()		const override final;
		virtual uint64_t GetAlignment()	const override final;
		virtual uint64_t GetNative()	const override final;

		VkBuffer GetNativeVK() const;
		VkMemoryRequirements GetMemoryRequirements() const;

	private:
		VkBuffer		m_Buffer			= VK_NULL_HANDLE;
		VkDeviceSize	m_Size				= 0;
		VmaAllocation	m_VmaAllocation		= VK_NULL_HANDLE;
		bool			m_Mapped			= false;
		void*			m_MappedPtr			= nullptr;
	};
}