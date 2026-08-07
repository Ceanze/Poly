#pragma once

#include <string_view>

namespace Poly
{
	enum class EFeaturePort
	{
		None,
		Color,
		Depth,
		Stencil
	};

	inline std::string_view ToSemanticName(EFeaturePort port)
	{
		switch (port)
		{
			case EFeaturePort::Color:   return "$Color";
			case EFeaturePort::Depth:   return "$Depth";
			case EFeaturePort::Stencil: return "$Stencil";
			default:                    return "";
		}
	}
}