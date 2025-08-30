#pragma once

namespace Poly
{
	enum class FFieldVisibility
	{
		UNKNOWN = 0,
		INPUT = FLAG(1),
		OUTPUT = FLAG(2),
		IN_OUT = INPUT | OUTPUT
	};
	ENABLE_BITMASK_OPERATORS(FFieldVisibility);
}