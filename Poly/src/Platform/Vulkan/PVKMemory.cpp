#include "polypch.h"
#include "PVKMemory.h"
#include "PVKInstance.h"
#include "PVKBuffer.h"
#include "VulkanCommon.h"

namespace Poly
{

	PVKMemory::PVKMemory() : currentOffset(0), memory(VK_NULL_HANDLE)
	{
	}

	PVKMemory::~PVKMemory()
	{
	}

	void PVKMemory::init(VkMemoryPropertyFlags memProp)
	{
		POLY_ASSERT(this->currentOffset != 0, "No buffers/images bound before allocation of memory!");

		uint32_t typeFilter = 0;
		for (auto buffer : this->bufferOffsets)
			typeFilter |= buffer.first->getMemReq().memoryTypeBits;
		//for (auto texture : this->textureOffsets)
		//	typeFilter |= texture.first->getMemReq().memoryTypeBits;

		uint32_t memoryTypeIndex = findMemoryType(PVKInstance::getPhysicalDevice(), typeFilter, memProp);

		VkMemoryAllocateInfo allocInfo = {};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = this->currentOffset;
		allocInfo.memoryTypeIndex = memoryTypeIndex;

		PVK_CHECK(vkAllocateMemory(PVKInstance::getDevice(), &allocInfo, nullptr, &this->memory), "Failed to allocate memory!");

		for (auto buffer : this->bufferOffsets)
			PVK_CHECK(vkBindBufferMemory(PVKInstance::getDevice(), buffer.first->getBuffer(), this->memory, buffer.second), "Failed to bind buffer memory!");

		//for (auto texture : this->textureOffsets)
		//	PVK_CHECK(vkBindImageMemory(Instance::get().getDevice(), texture.first->getVkImage(), this->memory, texture.second), "Failed to bind image memory!");
	}

	void PVKMemory::cleanup()
	{
		vkFreeMemory(PVKInstance::getDevice(), this->memory, nullptr);
	}

	void PVKMemory::bindBuffer(PVKBuffer& buffer)
	{
		this->bufferOffsets[&buffer] = this->currentOffset;
		this->currentOffset += static_cast<uint64_t>(buffer.getMemReq().size);
	}

	void PVKMemory::directTransfer(PVKBuffer& buffer, const void* data, uint64_t size, uint64_t bufferOffset)
	{
		uint64_t offset = this->bufferOffsets[&buffer];

		void* ptrGpu;
		PVK_CHECK(vkMapMemory(PVKInstance::getDevice(), this->memory, offset + bufferOffset, VK_WHOLE_SIZE, 0, &ptrGpu), "Failed to map memory for buffer!");
		memcpy(ptrGpu, data, size);
		vkUnmapMemory(PVKInstance::getDevice(), this->memory);
	}

	VkDeviceMemory PVKMemory::getMemory()
	{
		return this->memory;
	}

}