#pragma once

#include "polypch.h"
#include "PVKTypes.h"

#include <vulkan/vulkan.h>

namespace Poly
{
	class PVKBuffer
	{
	public:
		PVKBuffer();
		~PVKBuffer();

		void init(VkDeviceSize size, BufferUsage usage, const std::vector<uint32_t>& queueFamilyIndices);
		void cleanup();

		VkBuffer getNative() const;
		VkMemoryRequirements getMemReq() const;
		VkDeviceSize getSize() const;

	private:
		VkBuffer buffer;
		VkDeviceSize size;
	};
}