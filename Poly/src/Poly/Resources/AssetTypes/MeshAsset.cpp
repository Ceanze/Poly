#include "MeshAsset.h"

namespace Poly
{
	MeshAsset::MeshAsset(AssetID id, MeshRange meshRange)
	    : m_MeshRange(std::move(meshRange))
	{
		p_ID = id;
	}
	const MeshRange& MeshAsset::GetMeshRange() const
	{
		return m_MeshRange;
	}
} // namespace Poly