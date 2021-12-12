#pragma once
#include "Keys.h"

namespace Poly
{
	union KeyCode
	{
		KeyCode(EKey key) : key(key) {}
		KeyCode(EKey key, FKeyModifier modifier) : Key(key), Modifier(modifier) {}

		uint32 Value;
		struct
		{
			FKeyModifier Modifer	= 0,
			EKey Key				= 0,
		};
	};
}