#pragma once

#include "Poly/Resources/AssetHandle.h"

namespace Poly
{
	class MaterialAsset;

	struct MaterialComponent
	{
		AssetHandle<MaterialAsset> MaterialHandle;
	};
} // namespace Poly