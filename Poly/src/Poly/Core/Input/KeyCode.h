#pragma once
#include "Keys.h"

namespace Poly
{
	union KeyCode
	{
		KeyCode(EKey key) : Key(key), Modifier(FKeyModifier::NONE) {}
		KeyCode(EKey key, FKeyModifier modifier) : Key(key), Modifier(modifier) {}
		bool operator==(const KeyCode& other) const { return Value == other.Value; }

		uint32 Value;
		struct
		{
			FKeyModifier Modifier;
			EKey Key;
		};
	};
}

template<>
struct std::hash<Poly::KeyCode>
{
	std::size_t operator()(const Poly::KeyCode& kc) const noexcept
	{
		return std::hash<uint32>()(kc.Value);
	}
};