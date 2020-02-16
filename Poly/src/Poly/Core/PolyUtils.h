#pragma once

#include "Logger.h"
#include <stdexcept>

// PVK prefix is used for vulkan specific macros
// POLY prefix is used for general macros

// Check if a native vulkan function was successful. F is function, E is error message.
#define PVK_CHECK(F, E) if(F != VK_SUCCESS) { POLY_CORE_ERROR(E); throw std::runtime_error(E); }

// Cleanup of vectors of PVK types
#define PVK_VEC_CLEANUP(V) for (auto& e : V) { e.cleanup(); }