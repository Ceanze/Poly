#include "polypch.h"
#include "PVKBuffer.h"
#include "PVKInstance.h"

namespace Poly
{

	PVKBuffer::PVKBuffer()
	{
	}

	PVKBuffer::~PVKBuffer()
	{
	}

	void PVKBuffer::Init(VkDeviceSize size, BufferUsage usage, const std::vector<uint32_t>& queueFamilyIndices, VmaMemoryUsage memoryUsage)
	{
		m_Size = size;

		VkBufferCreateInfo createInfo = {};
		createInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		createInfo.size = size;
		createInfo.usage = (VkBufferUsageFlags)usage;

		if (queueFamilyIndices.size() > 1)
			createInfo.sharingMode = VK_SHARING_MODE_CONCURRENT;
		else
			createInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		createInfo.queueFamilyIndexCount = static_cast<uint32_t>(queueFamilyIndices.size());
		createInfo.pQueueFamilyIndices = queueFamilyIndices.data();

		VmaAllocationCreateInfo allocInfo = {};
		allocInfo.usage = memoryUsage;

		PVK_CHECK(vmaCreateBuffer(PVKInstance::GetAllocator(), &createInfo, &allocInfo, &m_Buffer, &m_VmaAllocation, nullptr), "Failed to create buffer using VMA");
	}

	void PVKBuffer::Init(const BufferDesc* pDesc)
	{

	}

	void PVKBuffer::Cleanup()
	{
		if (m_Mapped)
			Unmap();

		//vkDestroyBuffer(PVKInstance::getDevice(), this->buffer, nullptr);
		vmaDestroyBuffer(PVKInstance::GetAllocator(), m_Buffer, m_VmaAllocation);
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
		return m_Size;
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

	VkMemoryRequirements PVKBuffer::GetMemoryRequirements() const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(PVKInstance::GetDevice(), m_Buffer, &memRequirements);

		return memRequirements;
	}
}