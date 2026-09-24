#pragma once

#include "Poly/Resources/AssetHandle.h"
#include "Poly/Resources/IAssetPool.h"

namespace Poly
{
	template<typename AssetType>
	class AssetPool : public IAssetPool
	{
	public:
		AssetHandle<AssetType> Emplace(AssetID id, AssetType&& asset)
		{
			uint32 index;
			if (!m_FreeIndices.empty())
			{
				index = m_FreeIndices.back();
				m_FreeIndices.pop_back();
				m_Assets[index].Asset = std::move(asset);
				m_Assets[index].ID    = id;
			}
			else
			{
				index = (uint32)m_Assets.size();
				m_Assets.push_back(SlotData{std::move(asset), id, 0});
			}

			m_IDToIndex[id] = index;

			return AssetHandle<AssetType>(index, m_Assets[index].Generation);
		}

		AssetType* Resolve(AssetHandle<AssetType> handle)
		{
			if (!handle.IsValid())
				return nullptr;

			const uint32 index = handle.GetIndex();
			if (index >= m_Assets.size())
			{
				POLY_CORE_ERROR("Asset handle {} index is out of range for asset type. Index from handle: {}, max index allowed: {}", handle.Get(), index, m_Assets.size() - 1);
				return nullptr;
			}

			SlotData&    slot       = m_Assets[index];
			const uint32 generation = handle.GetGeneration();
			if (generation > slot.Generation)
			{
				POLY_CORE_WARN("Asset handle {} is out-of-date. Generation from handle: {}, current generation: {}", handle.Get(), generation, slot.Generation);
				return nullptr;
			}

			return &slot.Asset;
		}

		AssetHandle<AssetType> GetHandle(AssetID id) const
		{
			auto it = m_IDToIndex.find(id);
			if (it == m_IDToIndex.end())
				return AssetHandle<AssetType>();
			return AssetHandle<AssetType>(it->second, m_Assets[it->second].Generation);
		}

		bool Contains(AssetID id) override
		{
			return m_IDToIndex.contains(id);
		}

		void Erase(AssetID id) override
		{
			auto itr = m_IDToIndex.find(id);
			if (itr == m_IDToIndex.end())
				return;

			uint32 idx = itr->second;
			m_Assets[idx].Generation++;
			m_Assets[idx].Asset = AssetType{};
			m_FreeIndices.push_back(idx);
			m_IDToIndex.erase(itr);
		}

		void Erase(AssetHandle<AssetType> handle)
		{
			if (!handle.IsValid())
				return;

			const uint32 index = handle.GetIndex();
			if (index >= m_Assets.size())
				return;

			SlotData& slot = m_Assets[index];
			if (handle.GetGeneration() != slot.Generation)
				return; // Stale handle, already unloaded/reused

			Erase(slot.ID);
		}

	private:
		struct SlotData
		{
			AssetType Asset;
			AssetID   ID;
			uint32    Generation;
		};

		std::vector<SlotData>               m_Assets;
		std::vector<uint32>                 m_FreeIndices;
		std::unordered_map<AssetID, uint32> m_IDToIndex;
	};
} // namespace Poly