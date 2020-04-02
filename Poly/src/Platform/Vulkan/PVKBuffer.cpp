#include "polypch.h"
#include "PVKBuffer.h"
#include "PVKInstance.h"

namespace Poly
{

	PVKBuffer::PVKBuffer() : buffer(VK_NULL_HANDLE), size(0)
	{
	}

	PVKBuffer::~PVKBuffer()
	{
	}

	void PVKBuffer::init(VkDeviceSize size, BufferUsage usage, const std::vector<uint32_t>& queueFamilyIndices)
	{
		this->size = size;

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

		PVK_CHECK(vkCreateBuffer(PVKInstance::getDevice(), &createInfo, nullptr, &this->buffer), "Failed to create buffer!");
	}

	void PVKBuffer::cleanup()
	{
		vkDestroyBuffer(PVKInstance::getDevice(), this->buffer, nullptr);
	}

	VkBuffer PVKBuffer::getNative() const
	{
		return this->buffer;
	}

	VkMemoryRequirements PVKBuffer::getMemReq() const
	{
		VkMemoryRequirements memRequirements;
		vkGetBufferMemoryRequirements(PVKInstance::getDevice(), this->buffer, &memRequirements);

		return memRequirements;
	}

	VkDeviceSize PVKBuffer::getSize() const
	{
		return this->size;
	}

}