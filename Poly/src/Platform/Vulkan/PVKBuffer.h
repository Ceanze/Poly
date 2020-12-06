#pragma once

#include "polypch.h"
#include "PVKTypes.h"

#include "VmaInclude.h"

namespace Poly
{
	class PVKBuffer
	{
	public:
		PVKBuffer();
		~PVKBuffer();

		void Init(VkDeviceSize size, BufferUsage usage, const std::vector<uint32_t>& queueFamilyIndices, VmaMemoryUsage memoryUsage);
		void Cleanup();

		void* Map();
		void TransferData(const void* data, const size_t size);
		void Unmap();

		VkBuffer GetNative() const;
		VkMemoryRequirements GetMemReq() const;
		VkDeviceSize GetSize() const;

	private:
		VkBuffer		m_Buffer			= VK_NULL_HANDLE;
		VkDeviceSize	m_Size				= 0;
		VmaAllocation	m_VmaAllocation		= VK_NULL_HANDLE;
		bool			m_Mapped			= false;
		void*			m_MappedPtr			= nullptr;
	};
}