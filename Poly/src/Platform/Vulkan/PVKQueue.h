#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{
	struct PVKQueue
	{
		VkQueue queue				= VK_NULL_HANDLE;
		uint32_t queueIndex			= 0;
		uint32_t queueFamilyIndex	= 0;
	};

}