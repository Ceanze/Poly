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

		static void RegisterImporter(Unique<IAssetImporter> pImporter);

	private:
		static IAssetImporter* GetImporter(std::string_view vfsPath);

		inline static AssetRegistry                                    m_Registry;
		inline static std::unordered_map<std::string, IAssetImporter*> m_ExtensionToImporter;
		inline static std::vector<Unique<IAssetImporter>>              m_Importers;
	};
} // namespace Poly