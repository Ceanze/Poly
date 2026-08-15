#include "SceneAsset.h"

namespace Poly
{
	SceneAsset::SceneAsset(AssetID id)
	{
		p_ID = id;
	}

	const AssetHandle<MeshAsset>& SceneAsset::GetMeshAsset(uint32 index) const
	{
		if (index >= m_Meshes.size())
		{
			POLY_CORE_FATAL("Invalid index used to get mesh asset - must use indices from Node/Renderable");
			return m_Meshes.front();
		}

		return m_Meshes[index];
	}

	const AssetHandle<MaterialAsset>& SceneAsset::GetMaterialAsset(uint32 index) const
	{
		if (index >= m_Materials.size())
		{
			POLY_CORE_FATAL("Invalid index used to get material asset - must use indices from Node/Renderable");
			return m_Materials.front();
		}

		return m_Materials[index];
	}

	uint32 SceneAsset::AddNode(Node&& node)
	{
		uint32 index = m_Nodes.size();
		m_Nodes.emplace_back(std::move(node));
		return index;
	}

	uint32 SceneAsset::AddMeshAsset(AssetHandle<MeshAsset> meshAsset)
	{
		uint32 index = m_Meshes.size();
		m_Meshes.emplace_back(std::move(meshAsset));
		return index;
	}

	uint32 SceneAsset::AddMaterialAsset(AssetHandle<MaterialAsset> materialAsset)
	{
		uint32 index = m_Materials.size();
		m_Materials.emplace_back(std::move(materialAsset));
		return index;
	}

	void SceneAsset::SetRootNode(uint32 nodeIndex)
	{
		m_RootNodeIndex = nodeIndex;
	}
} // namespace Poly