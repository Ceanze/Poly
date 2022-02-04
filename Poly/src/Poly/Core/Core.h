#pragma once

#include <memory>

typedef unsigned char	byte;
typedef uint8_t			uint8;
typedef uint16_t		uint16;
typedef uint32_t		uint32;
typedef uint64_t		uint64;

namespace Poly
{
	// Custom names for the unique_ptr and shared_ptr

	template<typename T>
	using Unique = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Unique<T> CreateUnique(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	template<typename T>
	using Ref = std::shared_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Ref<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	#define CLASS_REMOVE_COPY(Typename) \
		Typename(const Typename&) = delete; \
		Typename& operator=(const Typename&) = delete \

	#define CLASS_REMOVE_MOVE(Typename) \
		Typename(Typename&&) = delete; \
		Typename& operator=(Typename&&) = delete \

	#define CLASS_STATIC(Typename) \
		CLASS_REMOVE_COPY(Typename); \
		CLASS_REMOVE_MOVE(Typename); \
		Typename() = delete; \
		~Typename() = delete

	#define CLASS_ABSTRACT(Typename) \
		CLASS_REMOVE_COPY(Typename); \
		CLASS_REMOVE_MOVE(Typename); \
		Typename() = default; \
		virtual ~Typename() = default
}