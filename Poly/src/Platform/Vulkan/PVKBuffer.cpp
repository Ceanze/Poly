#include "polypch.h"
#include "PVKBuffer.h"
#include "PVKInstance.h"

namespace Poly
{
	PVKBuffer::~PVKBuffer()
	{
		if (m_Mapped)
			Unmap();

		//vkDestroyBuffer(PVKInstance::getDevice(), this->buffer, nullptr);
		vmaDestroyBuffer(PVKInstance::GetAllocator(), m_Buffer, m_VmaAllocation);
	}

	void PVKBuffer::Init(const BufferDesc* pDesc)
	{
		p_BufferDesc = *pDesc;

		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = pDesc->Size;
		createInfo.usage = ConvertBufferUsageVK(pDesc->BufferUsage);
		createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE; // Only one queue supported at a time for now
		createInfo.queueFamilyIndexCount = 1;
		createInfo.pQueueFamilyIndices = nullptr;
		createInfo.pNext = nullptr;

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = ConvertMemoryUsageVMA(pDesc->MemUsage);

		PVK_CHECK(vmaCreateBuffer(PVKInstance::GetAllocator(), &createInfo, &allocInfo, &m_Buffer, &m_VmaAllocation, nullptr), "Failed to create buffer using VMA");
	}

	void* PVKBuffer::Map()
	{
		vmaMapMemory(PVKInstance::GetAllocator(), m_VmaAllocation, &m_MappedPtr);
		m_Mapped = true;
		return m_MappedPtr;
	}

	void PVKBuffer::TransferData(const void* data, const size_t size)
	{
		void* ptr = Map();
		memcpy(ptr, data, size);
		Unmap();
	}

	void PVKBuffer::Unmap()
	{
		vmaUnmapMemory(PVKInstance::GetAllocator(), m_VmaAllocation);
		m_Mapped = false;
	}

	VkDeviceSize PVKBuffer::GetSize() const
	{
		return p_BufferDesc.Size;
	}

	uint64_t PVKBuffer::GetAlignment()	const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(PVKInstance::GetDevice(), m_Buffer, &memRequirements);

		return memRequirements.alignment;
	}

	uint64_t PVKBuffer::GetNative() const
	{
		return reinterpret_cast<uint64_t>(m_Buffer);
	}

	VkBuffer PVKBuffer::GetNativeVK() const
	{
		return m_Buffer;
	}
}