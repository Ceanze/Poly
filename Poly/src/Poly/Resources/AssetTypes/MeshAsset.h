#pragma once

#include "Poly/RenderGraph/BufferRange.h"
#include "Poly/Resources/AssetTypes/IAsset.h"

namespace Poly
{
	struct MeshRange
	{
		BufferRange Vertices;
		BufferRange Indices;
	};

	class MeshAsset : public IAsset
	{
	public:
		MeshAsset() = default;
		MeshAsset(AssetID id, MeshRange meshRange);

		const MeshRange& GetMeshRange() const;

	private:
		MeshRange m_MeshRange;
	};
} // namespace Poly