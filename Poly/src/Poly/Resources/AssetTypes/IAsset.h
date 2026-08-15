#pragma once

#include "Poly/Resources/AssetID.h"

namespace Poly
{
	class IAsset
	{
	public:
		virtual ~IAsset() = default;

		AssetID GetID() { return p_ID; }

	protected:
		AssetID p_ID;
	};
} // namespace Poly