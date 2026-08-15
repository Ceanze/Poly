#pragma once

#include "Poly/Resources/AssetHandle.h"
#include "Poly/Resources/AssetTypes/IAsset.h"

namespace Poly
{
	class MeshAsset;
	class MaterialAsset;

	class SceneAsset : public IAsset
	{
	public:
		struct Renderable
		{
			uint32 MeshIndex;
			uint32 MaterialIndex;
		};

		struct Node
		{
			std::string Name;
			glm::vec3   Translation = {0.0f, 0.0f, 0.0f};
			glm::quat   Orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
			glm::vec3   Scale       = {1.0f, 1.0f, 1.0f};

			std::vector<Renderable> Renderables;
			std::vector<uint32>     ChildrenIndices;
		};

		explicit SceneAsset(AssetID id);

		const AssetHandle<MeshAsset>&     GetMeshAsset(uint32 index) const;
		const AssetHandle<MaterialAsset>& GetMaterialAsset(uint32 index) const;

		uint32 AddNode(Node&& node);
		uint32 AddMeshAsset(AssetHandle<MeshAsset> meshAsset);
		uint32 AddMaterialAsset(AssetHandle<MaterialAsset> materialAsset);
		void   SetRootNode(uint32 nodeIndex);

	private:
		std::vector<AssetHandle<MeshAsset>>     m_Meshes;
		std::vector<AssetHandle<MaterialAsset>> m_Materials;
		std::vector<Node>                       m_Nodes;

		uint32 m_RootNodeIndex;
	};
} // namespace Poly