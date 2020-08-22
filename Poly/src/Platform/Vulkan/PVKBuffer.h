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

		void init(VkDeviceSize size, BufferUsage usage, const std::vector<uint32_t>& queueFamilyIndices, VmaMemoryUsage memoryUsage);
		void cleanup();

		void transferData(const void* data, const size_t size);

		VkBuffer getNative() const;
		VkMemoryRequirements getMemReq() const;
		VkDeviceSize getSize() const;

	private:
		VkBuffer buffer;
		VkDeviceSize size;
		VmaAllocation vmaAllocation;
	};
}