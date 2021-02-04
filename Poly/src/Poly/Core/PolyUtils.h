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

template<typename Enum>
inline bool EnumCheck(Enum e)
{
    return static_cast<std::underlying_type<Enum>::type>(e) > 0;
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator |(Enum lhs, Enum rhs)
{
    using underlying = typename std::underlying_type<Enum>::type;
    return static_cast<Enum> (
        static_cast<underlying>(lhs) |
        static_cast<underlying>(rhs)
        );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator &(Enum lhs, Enum rhs)
{
    return static_cast<Enum> (
        static_cast<std::underlying_type<Enum>::type>(lhs)&
        static_cast<std::underlying_type<Enum>::type>(rhs)
        );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator ^(Enum lhs, Enum rhs)
{
    return static_cast<Enum> (
        static_cast<std::underlying_type<Enum>::type>(lhs) ^
        static_cast<std::underlying_type<Enum>::type>(rhs)
        );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator ~(Enum rhs)
{
    return static_cast<Enum> (
        ~static_cast<std::underlying_type<Enum>::type>(rhs)
        );
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator |=(Enum & lhs, Enum rhs)
{
    lhs = static_cast<Enum> (
        static_cast<std::underlying_type<Enum>::type>(lhs) |
        static_cast<std::underlying_type<Enum>::type>(rhs)
        );

    return lhs;
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator &=(Enum & lhs, Enum rhs)
{
    lhs = static_cast<Enum> (
        static_cast<std::underlying_type<Enum>::type>(lhs)&
        static_cast<std::underlying_type<Enum>::type>(rhs)
        );

    return lhs;
}

template<typename Enum>
typename std::enable_if<EnableBitMaskOperators<Enum>::enable, Enum>::type
operator ^=(Enum & lhs, Enum rhs)
{
    lhs = static_cast<Enum> (
        static_cast<std::underlying_type<Enum>::type>(lhs)^
        static_cast<std::underlying_type<Enum>::type>(rhs)
        );

    return lhs;
}

#define ENABLE_BITMASK_OPERATORS(type)\
template<>\
struct EnableBitMaskOperators<type>\
{\
static const bool enable = true;\
};