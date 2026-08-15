#pragma once

#include "Poly/Model/Mesh.h" // TODO: Remove, contains MeshRange for now
#include "Poly/Resources/AssetTypes/IAsset.h"

namespace Poly
{
	class MeshAsset : public IAsset
	{
	public:
		MeshAsset(AssetID id, MeshRange meshRange);

		const MeshRange& GetMeshRange() const;

	private:
		MeshRange m_MeshRange;
	};
} // namespace Poly