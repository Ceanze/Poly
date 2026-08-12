#pragma once

#include "Poly/Resources/AssetID.h"

namespace Poly
{
	class IAsset
	{
	public:
		virtual ~IAsset() = default;

		virtual AssetID GetID() = 0;
	};
} // namespace Poly