#pragma once

#include "Poly/Resources/AssetID.h"

namespace Poly
{
	class IAssetPool
	{
	public:
		virtual ~IAssetPool() = default;

		virtual bool Contains(AssetID id) = 0;

		virtual void Erase(AssetID id) = 0;
	};
} // namespace Poly