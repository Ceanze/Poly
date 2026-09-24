#include "AssetHandler.h"

#include "Poly/Resources/AssetTypes/MaterialAsset.h"
#include "Poly/Resources/AssetTypes/MeshAsset.h"
#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/Resources/AssetTypes/TextureAsset.h"
#include "Poly/Resources/Importers/IAssetImporter.h"
#include "Poly/Resources/PathUtils.h"

namespace Poly
{
	std::vector<Unique<IAssetImporter>> AssetHandler::m_Importers;

	void AssetHandler::Init()
	{
	}

	void AssetHandler::Release()
	{
		m_Registry.UnloadAll();
		m_ExtensionToImporter.clear();
		m_Importers.clear();
	}

	void AssetHandler::ScanAssets()
	{
		// TODO: Scan only extensions that are from registered importers
		m_Registry.ScanAssets();
	}

	IAssetImporter* AssetHandler::GetImporter(std::string_view vfsPath)
	{
		std::string extension = PathUtils::GetExtension(vfsPath);
		if (extension.empty())
		{
			POLY_CORE_WARN("No extension found for {}", vfsPath);
			return nullptr;
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
		AssetID id(vfsPath);
		if (m_Registry.IsLoaded<AssetType>(id))
			return m_Registry.GetHandle<AssetType>(id);

		// Sub-assets ("foo.gltf#mesh_0") only exist as a product of importing their source asset
		const std::string_view sourcePath = PathUtils::GetSourcePath(vfsPath);
		const bool             isSubAsset = sourcePath.size() != vfsPath.size();

		// Re-importing an already imported source would duplicate all of its sub-assets
		if (isSubAsset && m_Registry.IsLoaded(AssetID(sourcePath)))
		{
			POLY_CORE_WARN("Cannot load sub-asset {}, source {} is already imported but does not contain it", vfsPath, sourcePath);
			return AssetHandle<AssetType>();
		}

		IAssetImporter* pImporter = GetImporter(sourcePath);
		if (!pImporter)
			return AssetHandle<AssetType>();

		if (!pImporter->Import(sourcePath, m_Registry))
			return AssetHandle<AssetType>();

		AssetHandle<AssetType> handle = m_Registry.GetHandle<AssetType>(id);
		if (!handle.IsValid())
			POLY_CORE_WARN("Importing {} did not produce the requested asset {}", sourcePath, vfsPath);

		return handle;
	}

	template<typename AssetType>
	AssetHandle<AssetType> AssetHandler::Load(AssetID assetID)
	{
		const std::string& path = m_Registry.ResolvePath(assetID);
		if (path.empty())
			return AssetHandle<AssetType>();

		return Load<AssetType>(path);
	}

	template<typename AssetType>
	std::future<AssetHandle<AssetType>> AssetHandler::LoadAsync(std::string_view vfsPath)
	{
		return LoadAsync<AssetType>(AssetID(vfsPath));
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
		m_Registry.Unload<AssetType>(handle);
	}

	template<typename AssetType>
	std::string AssetHandler::GetPath(AssetHandle<AssetType> handle)
	{
		AssetType* pAsset = Resolve(handle);
		if (!pAsset)
			return std::string();

		return m_Registry.ResolvePath(pAsset->GetID());
	}

	// Instantiate the supported types (this allows us to hide the definition from the header)
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

	template std::string AssetHandler::GetPath<MeshAsset>(AssetHandle<MeshAsset>);
	template std::string AssetHandler::GetPath<SceneAsset>(AssetHandle<SceneAsset>);
	template std::string AssetHandler::GetPath<TextureAsset>(AssetHandle<TextureAsset>);
	template std::string AssetHandler::GetPath<MaterialAsset>(AssetHandle<MaterialAsset>);
} // namespace Poly