#pragma once

#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/Resources/Importers/IAssetImporter.h"

class aiNode;
class aiScene;
class aiMaterial;

namespace Poly
{
	class SceneAssetImporter : public IAssetImporter
	{
	public:
		// IAssetImporter
		std::vector<std::string> GetSupportedExtensions() const;
		bool                     Import(std::string_view vfsPath, AssetRegistry& registry);

	private:
		MaterialAsset    LoadMaterial(AssetID assetID, const aiMaterial* pMaterial, std::string_view vfsPath);
		MeshAsset        LoadMesh(AssetID assetID, const aiMesh* pMesh);
		SceneAsset::Node ImportNode(SceneAsset& sceneAsset, const aiScene* pScene, const aiNode* pNode);
	};
} // namespace Poly