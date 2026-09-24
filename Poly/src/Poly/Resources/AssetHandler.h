#pragma once

#include "Poly/Resources/AssetHandle.h"
#include "Poly/Resources/AssetID.h"
#include "Poly/Resources/AssetRegistry.h"

#include <future>

namespace Poly
{
	class IAssetImporter;

	// TODO: Rename to AssetManager once the old asset manager is removed
	class AssetHandler
	{
	public:
		CLASS_STATIC(AssetHandler);

		static void Init();
		static void Release();

		/**
		 * Scans all the mounted folders in the VFS for assets, registering their assetIds and paths.
		 * Also scans and imports any existing .meta files.
		 * All importers that adds supported extensions must be registered before scanning, as non-supported files are ignored.
		 */
		static void ScanAssets();

		template<typename AssetType>
		static AssetHandle<AssetType> Load(std::string_view vfsPath);

		template<typename AssetType>
		static AssetHandle<AssetType> Load(AssetID assetID);

		template<typename AssetType>
		static std::future<AssetHandle<AssetType>> LoadAsync(std::string_view vfsPath);

		template<typename AssetType>
		static std::future<AssetHandle<AssetType>> LoadAsync(AssetID assetID);

		template<typename AssetType>
		static AssetType* Resolve(AssetHandle<AssetType> handle);

		template<typename AssetType>
		static void Unload(AssetHandle<AssetType> handle);

		/**
		 * @param handle - handle of a loaded asset
		 * @return VFS path of the asset, sub-assets include their #fragment. Empty if the handle is invalid or the path is unknown
		 */
		template<typename AssetType>
		static std::string GetPath(AssetHandle<AssetType> handle);

		static void RegisterImporter(Unique<IAssetImporter> pImporter);

	private:
		static IAssetImporter* GetImporter(std::string_view vfsPath);

		inline static AssetRegistry                                    m_Registry;
		inline static std::unordered_map<std::string, IAssetImporter*> m_ExtensionToImporter;
		static std::vector<Unique<IAssetImporter>>                     m_Importers;
	};
} // namespace Poly