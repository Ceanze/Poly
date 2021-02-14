#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{
	struct PVKQueue
	{
		PVKQueue() : queue(VK_NULL_HANDLE), queueIndex(0) {};

		VkQueue queue;
		uint32_t queueIndex;
		uint32_t queueFamilyIndex;
	};

}