#pragma once

#include <vulkan/vulkan.h>

namespace Poly
{

	struct PVKQueue
	{
		VkQueue queue;
		uint32_t queueIndex;
	};

}