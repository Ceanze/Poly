#pragma once

#include "polypch.h"

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKBuffer
	{
	public:
		PVKBuffer();
		~PVKBuffer();

		void init(VkDeviceSize size, VkBufferUsageFlags usage, const std::vector<uint32_t>& queueFamilyIndices);
		void cleanup();

		VkBuffer getBuffer() const;
		VkMemoryRequirements getMemReq() const;
		VkDeviceSize getSize() const;

	private:
		VkBuffer buffer;
		VkDeviceSize size;
	};
}