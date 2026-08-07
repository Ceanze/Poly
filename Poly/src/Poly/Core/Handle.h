#pragma once

#include "Core.h"

#include <functional>

namespace Poly
{
	template<typename Tag>
	class Handle
	{
	public:
		static constexpr uint32 INDEX_BITS      = 24;
		static constexpr uint32 GENERATION_BITS = 8;
		static constexpr uint32 INDEX_MASK      = (1u << INDEX_BITS) - 1;
		static constexpr uint32 INVALID_PACKED  = ~0u;

		constexpr Handle() = default;
		Handle(uint32 index, uint32 generation)
		    : m_Packed(((generation & 0xFFu) << INDEX_BITS) | (index & INDEX_MASK))
		{}

		Handle(uint32 packed)
		    : m_Packed(packed)
		{}

		uint32 GetIndex() const { return m_Packed & INDEX_MASK; }
		uint32 GetGeneration() const { return m_Packed >> INDEX_BITS; }
		bool   IsValid() const { return m_Packed != INVALID_PACKED; }

		uint32 Get() const { return m_Packed; }

		bool
		operator==(const Handle& other) const
		{
			return m_Packed == other.m_Packed;
		}
		bool operator!=(const Handle& other) const { return m_Packed != other.m_Packed; }

	private:
		uint32 m_Packed = INVALID_PACKED;
	};
} // namespace Poly

namespace std
{
	template<typename Tag>
	struct hash<Poly::Handle<Tag>>
	{
		size_t operator()(const Poly::Handle<Tag>& handle) const
		{
			return std::hash<uint32>()(handle.GetIndex()) ^ (std::hash<uint32>()(handle.GetGeneration()) << 1);
		}
	};
} // namespace std
