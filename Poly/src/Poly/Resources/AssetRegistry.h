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

		std::string ResolvePath(AssetID assetID) const;

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
		void Emplace(AssetID assetID, AssetType&& assetType)
		{
			return GetOrCreatePool<AssetType>().Emplace(assetID, std::move(asset));
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