#include "polypch.h"
#include "PVKMemory.h"
#include "PVKInstance.h"
#include "PVKBuffer.h"
#include "VulkanCommon.h"
#include "PVKTexture.h"

namespace Poly
{

	PVKMemory::PVKMemory()
	{
	}

	PVKMemory::~PVKMemory()
	{
	}

	void PVKMemory::Init(MemoryPropery memProp)
	{
		POLY_ASSERT(m_CurrentOffset != 0, "No buffers/images bound before allocation of memory!");

		uint32_t typeFilter = 0;
		for (auto buffer : m_BufferOffsets)
			typeFilter |= buffer.first->GetMemReq().memoryTypeBits;
		for (auto texture : m_TextureOffsets)
			typeFilter |= texture.first->GetMemoryRequirements().memoryTypeBits;

		uint32_t memoryTypeIndex = findMemoryType(PVKInstance::GetPhysicalDevice(), typeFilter, (VkMemoryPropertyFlags)memProp);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = m_CurrentOffset;
		allocInfo.memoryTypeIndex = memoryTypeIndex;

		PVK_CHECK(vkAllocateMemory(PVKInstance::GetDevice(), &allocInfo, nullptr, &m_Memory), "Failed to allocate memory!");

		for (auto buffer : m_BufferOffsets)
			PVK_CHECK(vkBindBufferMemory(PVKInstance::GetDevice(), buffer.first->GetNative(), m_Memory, buffer.second), "Failed to bind buffer memory!");

		for (auto texture : m_TextureOffsets)
			PVK_CHECK(vkBindImageMemory(PVKInstance::GetDevice(), texture.first->GetImage().GetNative(), m_Memory, texture.second), "Failed to bind image memory!");
	}

	void PVKMemory::Cleanup()
	{
		vkFreeMemory(PVKInstance::GetDevice(), m_Memory, nullptr);
	}

	void PVKMemory::BindBuffer(PVKBuffer& buffer)
	{
		m_BufferOffsets[&buffer] = m_CurrentOffset;
		m_CurrentOffset += static_cast<uint64_t>(buffer.GetMemReq().size);
	}

	void PVKMemory::BindTexture(PVKTexture& texture)
	{
		m_TextureOffsets[&texture] = m_CurrentOffset;
		m_CurrentOffset += static_cast<uint64_t>(texture.GetMemoryRequirements().size);
	}

	void PVKMemory::DirectTransfer(PVKBuffer& buffer, const void* pData, uint64_t size, uint64_t bufferOffset)
	{
		uint64_t offset = m_BufferOffsets[&buffer];

		void* ptrGpu;
		PVK_CHECK(vkMapMemory(PVKInstance::GetDevice(), m_Memory, offset + bufferOffset, VK_WHOLE_SIZE, 0, &ptrGpu), "Failed to map memory for buffer!");
		memcpy(ptrGpu, pData, size);
		vkUnmapMemory(PVKInstance::GetDevice(), m_Memory);
	}

}