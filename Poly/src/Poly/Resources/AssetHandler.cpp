#include "Poly/Resources/AssetHandler.h"

#include "AssetHandler.h"
#include "Poly/Resources/Importers/IAssetImporter.h"
#include "Poly/Resources/PathUtils.h"

namespace Poly
{
	class MeshAsset;
	class SceneAsset;
	class TextureAsset;
	class MaterialAsset;

	void AssetHandler::Init()
	{
	}

	void AssetHandler::Release()
	{
	}

	IAssetImporter* AssetHandler::GetImporter(std::string_view vfsPath)
	{
		std::string extension = PathUtils::GetExtension(vfsPath);
		if (extension.empty())
		{
			POLY_CORE_WARN("No extension found for {}", vfsPath);
			return;
		}

		auto it = m_ExtensionToImporter.find(extension);
		return it != m_ExtensionToImporter.end() ? it->second : nullptr;
	}

	void AssetHandler::RegisterImporter(Unique<IAssetImporter> pImporter)
	{
		IAssetImporter* pRawImporter = pImporter.get();
		m_Importers.emplace_back(std::move(pImporter));

		for (const std::string& extension : pRawImporter->GetSupportedExtensions())
		{
			if (m_ExtensionToImporter.contains(extension))
			{
				POLY_CORE_WARN("Importer with supported extension '{}' has already been imported, skipping", extension);
				continue;
			}

			m_ExtensionToImporter[extension] = pRawImporter;
		}
	}

	template<typename AssetType>
	AssetHandle<AssetType> AssetHandler::Load(std::string_view vfsPath)
	{
		return Load<AssetType>(AssetID(vfsPath));
	}

	template<typename AssetType>
	AssetHandle<AssetType> AssetHandler::Load(AssetID assetID)
	{
		return AssetHandle<AssetType>();
	}

	template<typename AssetType>
	std::future<AssetHandle<AssetType>> AssetHandler::LoadAsync(std::string_view vfsPath)
	{
		return LoadAsync(AssetID(vfsPath));
	}

	template<typename AssetType>
	std::future<AssetHandle<AssetType>> AssetHandler::LoadAsync(AssetID assetID)
	{
		return std::future<AssetHandle<AssetType>>();
	}

	template<typename AssetType>
	AssetType* AssetHandler::Resolve(AssetHandle<AssetType> handle)
	{
		return m_Registry.Resolve(handle);
	}

	template<typename AssetType>
	void AssetHandler::Unload(AssetHandle<AssetType> handle)
	{
	}

	// Instansiate the supported types (this allows us to hide the definition from the header)
	template AssetHandle<MeshAsset>     AssetHandler::Load<MeshAsset>(std::string_view);
	template AssetHandle<SceneAsset>    AssetHandler::Load<SceneAsset>(std::string_view);
	template AssetHandle<TextureAsset>  AssetHandler::Load<TextureAsset>(std::string_view);
	template AssetHandle<MaterialAsset> AssetHandler::Load<MaterialAsset>(std::string_view);

	template AssetHandle<MeshAsset>     AssetHandler::Load<MeshAsset>(AssetID);
	template AssetHandle<SceneAsset>    AssetHandler::Load<SceneAsset>(AssetID);
	template AssetHandle<TextureAsset>  AssetHandler::Load<TextureAsset>(AssetID);
	template AssetHandle<MaterialAsset> AssetHandler::Load<MaterialAsset>(AssetID);

	template std::future<AssetHandle<MeshAsset>>     AssetHandler::LoadAsync<MeshAsset>(std::string_view);
	template std::future<AssetHandle<SceneAsset>>    AssetHandler::LoadAsync<SceneAsset>(std::string_view);
	template std::future<AssetHandle<TextureAsset>>  AssetHandler::LoadAsync<TextureAsset>(std::string_view);
	template std::future<AssetHandle<MaterialAsset>> AssetHandler::LoadAsync<MaterialAsset>(std::string_view);

	template std::future<AssetHandle<MeshAsset>>     AssetHandler::LoadAsync<MeshAsset>(AssetID);
	template std::future<AssetHandle<SceneAsset>>    AssetHandler::LoadAsync<SceneAsset>(AssetID);
	template std::future<AssetHandle<TextureAsset>>  AssetHandler::LoadAsync<TextureAsset>(AssetID);
	template std::future<AssetHandle<MaterialAsset>> AssetHandler::LoadAsync<MaterialAsset>(AssetID);

	template MeshAsset*     AssetHandler::Resolve<MeshAsset>(AssetHandle<MeshAsset>);
	template SceneAsset*    AssetHandler::Resolve<SceneAsset>(AssetHandle<SceneAsset>);
	template TextureAsset*  AssetHandler::Resolve<TextureAsset>(AssetHandle<TextureAsset>);
	template MaterialAsset* AssetHandler::Resolve<MaterialAsset>(AssetHandle<MaterialAsset>);

	template void AssetHandler::Unload<MeshAsset>(AssetHandle<MeshAsset>);
	template void AssetHandler::Unload<SceneAsset>(AssetHandle<SceneAsset>);
	template void AssetHandler::Unload<TextureAsset>(AssetHandle<TextureAsset>);
	template void AssetHandler::Unload<MaterialAsset>(AssetHandle<MaterialAsset>);
} // namespace Poly