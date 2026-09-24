#pragma once

#include "Poly/Resources/AssetHandle.h"
#include "Poly/Resources/AssetID.h"
#include "Poly/Resources/AssetPool.h"

#include <typeindex>

namespace Poly
{
	class IAssetPool;

	class AssetRegistry
	{
	public:
		AssetRegistry() = default;

		void ScanAssets();

		const std::string& ResolvePath(AssetID assetID) const;

		/**
		 * Registers the VFS path of an asset, e.g. a sub-asset created during import.
		 * @param assetID - ID of the asset
		 * @param vfsPath - VFS path the asset ID was derived from
		 */
		void RegisterPath(AssetID assetID, std::string_view vfsPath);

		template<typename AssetType>
		AssetHandle<AssetType> GetHandle(AssetID id) const
		{
			return GetPool<AssetType>() ? GetPool<AssetType>()->GetHandle(id) : AssetHandle<AssetType>();
		}

		template<typename AssetType>
		AssetType* Resolve(AssetHandle<AssetType> handle) const
		{
			return GetPool<AssetType>() ? GetPool<AssetType>()->Resolve(handle) : nullptr;
		}

		template<typename AssetType>
		AssetHandle<AssetType> Emplace(AssetID assetID, AssetType&& asset)
		{
			return GetOrCreatePool<AssetType>().Emplace(assetID, std::move(asset));
		}

		template<typename AssetType>
		bool IsLoaded(AssetID id) const
		{
			return GetPool<AssetType>() ? GetPool<AssetType>()->Contains(id) : false;
		}

		bool IsLoaded(AssetID id) const
		{
			for (auto& [type, pool] : m_Pools)
				if (pool->Contains(id))
					return true;
			return false;
		}

		template<typename AssetType>
		void Unload(AssetHandle<AssetType> handle)
		{
			if (AssetPool<AssetType>* pool = GetPool<AssetType>())
				pool->Erase(handle);
		}

		void UnloadAll()
		{
			m_Pools.clear();
			m_IDToPath.clear();
		}

	private:
		template<typename AssetType>
		AssetPool<AssetType>& GetOrCreatePool()
		{
			auto& slot = m_Pools[std::type_index(typeid(AssetType))];
			if (!slot)
				slot = CreateUnique<AssetPool<AssetType>>();
			return static_cast<AssetPool<AssetType>&>(*slot);
		}

		template<typename AssetType>
		AssetPool<AssetType>* GetPool() const
		{
			auto it = m_Pools.find(std::type_index(typeid(AssetType)));
			return it != m_Pools.end() ? static_cast<AssetPool<AssetType>*>(it->second.get()) : nullptr;
		}

		std::unordered_map<std::type_index, Unique<IAssetPool>> m_Pools;

		std::unordered_map<AssetID, std::string> m_IDToPath; // TODO: Not needed for release runtime, only editor(?)
	};
} // namespace Poly