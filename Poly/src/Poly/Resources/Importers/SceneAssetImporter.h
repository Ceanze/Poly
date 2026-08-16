#pragma once

#include "Poly/Resources/AssetTypes/MaterialAsset.h"
#include "Poly/Resources/AssetTypes/MeshAsset.h"
#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/Resources/Importers/IAssetImporter.h"

struct aiNode;
struct aiScene;
struct aiMaterial;
struct aiMesh;

namespace Poly
{
	class SceneAssetImporter : public IAssetImporter
	{
	public:
		// IAssetImporter
		std::vector<std::string> GetSupportedExtensions() const;
		bool                     Import(std::string_view vfsPath, AssetRegistry& registry);

	private:
		MaterialAsset    LoadMaterial(const AssetID& assetID, const aiMaterial* pMaterial, std::string_view vfsPath);
		MeshAsset        LoadMesh(const AssetID& assetID, const aiMesh* pMesh);
		SceneAsset::Node ImportNode(SceneAsset& sceneAsset, const aiScene* pScene, const aiNode* pNode);
	};
} // namespace Poly