#pragma once

/*
	Include this file when using VMA.
	When this file is included vulkan is not needed to be included, as that is included in vk_mem_alloc.h
*/


//#define VMA_STATIC_VULKAN_FUNCTIONS 0
//#define VMA_DYNAMIC_VULKAN_FUNCTIONS 1

#define VMA_VULKAN_VERSION 1001000 // Vulkan 1.1

#pragma warning(push, 4)
#pragma warning(disable: 4127) // conditional expression is constant
#pragma warning(disable: 4100) // unreferenced formal parameter
#pragma warning(disable: 4189) // local variable is initialized but not referenced
#pragma warning(disable: 4324) // structure was padded due to alignment specifier

#include "vk_mem_alloc.h"

#pragma warning(pop)