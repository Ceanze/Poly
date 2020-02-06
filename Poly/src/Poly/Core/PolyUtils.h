#pragma once

#include "Logger.h"
#include <stdexcept>

// Check if a native vulkan function was successful. F is function, E is error message.
#define PVK_CHECK(F, E) if(F != VK_SUCCESS) { POLY_CORE_ERROR(E); throw std::runtime_error(E); }