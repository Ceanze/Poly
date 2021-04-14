#pragma once

#include "Logger.h"
#include <type_traits>

// PVK prefix is used for vulkan specific macros
// POLY prefix is used for general macros

// Check if a native vulkan function was successful. First param is function, second is error message.
#define PVK_CHECK(F, ...) if(F != VK_SUCCESS) { POLY_CORE_ERROR(__VA_ARGS__); assert(F); }

// Cleanup of vectors of PVK types
#define PVK_VEC_CLEANUP(V) for (auto& e : V) { e.Cleanup(); }

// Safe cleanup of PVK types
#define PVK_CLEANUP(handle, func) if (handle != VK_NULL_HANDLE) { func; }

#define POLY_VALIDATE(exp, ...) { if(!(exp)) {POLY_CORE_ERROR(__VA_ARGS__); } assert(exp); }

// Bit helpers
#define BIT(bit)	(1 << bit)
#define FLAG(bit)	BIT(bit)

// Binary operators for enum classes, enable using ENABLE_BINARY_OPERATIONS(EnumnType);
template<typename Enum>
struct EnableBitMaskOperators
{
	static const bool enable = false;
};

#define FLAG_CHECK(Enum, trueCase) if (EnumCheck(Enum)) trueCase

#define ENUM_CAST(e, t) static_cast<std::underlying_type_t<e>>(t)

#define ENABLE_BITMASK_OPERATORS(e)\
	inline e operator |(e lhs, e rhs)		{ return static_cast<e>(ENUM_CAST(e, lhs) | ENUM_CAST(e, rhs)); } \
	inline e& operator |=(e& lhs, e rhs)	{ lhs = lhs | rhs; return lhs; } \
	inline e operator &(e lhs, e rhs)		{ return static_cast<e>(ENUM_CAST(e, lhs) & ENUM_CAST(e, rhs)); } \
	inline e& operator &=(e& lhs, e rhs)	{ lhs = lhs & rhs; return lhs; } \
	inline e operator ^(e lhs, e rhs)		{ return static_cast<e>(ENUM_CAST(e, lhs) ^ ENUM_CAST(e, rhs)); } \
	inline e& operator ^=(e& lhs, e rhs)	{ lhs = lhs ^ rhs; return lhs; } \
	inline e operator ~(e lhs)				{ return static_cast<e>(~ENUM_CAST(e, lhs)); } \
	inline bool BitsSet(e val, e flag)		{ return ENUM_CAST(e, val & flag) > ENUM_CAST(e, 0); } \
	inline bool EnumCheck(e val)			{ return ENUM_CAST(e, val) > 0; }