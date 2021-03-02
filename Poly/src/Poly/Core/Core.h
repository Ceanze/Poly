#pragma once

#include <memory>

typedef uint32_t		uint32;
typedef uint64_t		uint64;
typedef uint8_t			uint8;
typedef unsigned char	byte;

namespace Poly
{
	// Custom names for the unique_ptr and shared_ptr

	template<typename T>
	using Scope = std::unique_ptr<T>;
	template<typename T, typename ... Args>
	constexpr Scope<T> CreateScope(Args&& ... args)
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