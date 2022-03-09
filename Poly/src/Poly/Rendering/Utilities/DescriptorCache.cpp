#include "DescriptorCache.h"

#include "Poly/Core/RenderAPI.h"
#include "Platform/API/DescriptorSet.h"

namespace Poly
{
	void DescriptorCache::Update(uint32 frameIndex)
	{
		if (m_DescriptorsToBeDeleted.contains(frameIndex))
			m_DescriptorsToBeDeleted[frameIndex].clear();
	}

	const DescriptorSet* DescriptorCache::GetDescriptorSetCopy(uint32 set, uint32 frameIndex, uint32 index, uint32 offset, uint32 segmentSize)
	{
		if (ValidateOffset(offset, segmentSize))
			offset = offset / segmentSize;
		else
			return;

		CacheKey key(frameIndex, set);
		if (!HasDescriptor(key, index, offset))
			return CreateDescriptor(key, index, offset).get();

		Ref<DescriptorSet> pOldDescriptor = m_Descriptors[key][index][offset];

		Ref<DescriptorSet> pNewDescriptor = RenderAPI::CreateDescriptorSetCopy(pOldDescriptor);
		m_Descriptors[key][index][offset] = pNewDescriptor;

		m_DescriptorsToBeDeleted[frameIndex].push_back(pOldDescriptor);

		return pNewDescriptor.get();
	}

	void DescriptorCache::SetPipelineLayout(PipelineLayout* pPipelineLayout)
	{
		m_pPipelineLayout = pPipelineLayout;
	}

	const DescriptorSet* DescriptorCache::GetDescriptorSet(uint32 set, uint32 frameIndex)
	{
		GetDescriptorSet(set, frameIndex, 0, 0, 0);
	}

	const DescriptorSet* DescriptorCache::GetDescriptorSet(uint32 set, uint32 frameIndex, uint32 index)
	{
		GetDescriptorSet(set, frameIndex, index, 0, 0);
	}

	const DescriptorSet* DescriptorCache::GetDescriptorSet(uint32 set, uint32 frameIndex, uint32 index, uint32 offset, uint32 segmentSize)
	{
		if (ValidateOffset(offset, segmentSize))
			offset = offset / segmentSize;
		else
			return;

		CacheKey key(frameIndex, set);
		if (!HasDescriptor(key, index, offset))
			return CreateDescriptor(key, index, offset).get();

		return m_Descriptors[key][index][offset].get();
	}

	Ref<DescriptorSet> DescriptorCache::CreateDescriptor(CacheKey key, uint32 index, uint32 offset)
	{
		Ref<DescriptorSet> pDescriptorSet = RenderAPI::CreateDescriptorSet(m_pPipelineLayout, key.Set);

		auto& indexVector = m_Descriptors[key];
		if (index >= indexVector.size())
			indexVector.resize(index);

		auto& offsetVector = indexVector[index];
		if (offset >= offsetVector.size())
			offsetVector.resize(offset);

		offsetVector[offset] = pDescriptorSet;
		return pDescriptorSet;
	}

	bool DescriptorCache::HasDescriptor(CacheKey key, uint32 index, uint32 offset)
	{
		if (!m_Descriptors.contains(key))
			return false;

		const auto& indexVector = m_Descriptors[key];
		if (index >= indexVector.size())
			return false;

		const auto& offsetVector = indexVector[index];
		if (offset >= offsetVector.size())
			return false;

		const Ref<DescriptorSet>& pDescriptorSet = offsetVector[offset];
		if (!pDescriptorSet)
			return false;

		return true;
	}

	bool DescriptorCache::ValidateOffset(uint32 offset, uint32 segmentSize)
	{
		if (offset > 0)
		{
			if (segmentSize == 0)
			{
				POLY_CORE_ERROR("Tried to get descriptor with offset {}, but segment size was zero. If offset is used then segment size must be larger than zero.",
								offset);
				return false;
			}
		}

		return true;
	}
}