#pragma once

#include "Poly/Resources/AssetHandle.h"

namespace Poly
{
	class MeshAsset;

	// TODO: Rename when MeshComponent is removed
	struct MeshAssetComponent
	{
		AssetHandle<MeshAsset> MeshHandle;
	};
}