#include "DescriptorCache.h"

#include "Poly/Core/RenderAPI.h"
#include "Platform/API/DescriptorSet.h"

namespace Poly
{
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
		if (offset > 0)
		{
			if (segmentSize == 0)
			{
				POLY_CORE_ERROR("Tried to get descriptor with set {}, frame index {}, index {} and offset {}, but segment size was zero. If offset is used then segment size must be larger than zero.",
								set, frameIndex, index, offset);
				return;
			}
			offset = offset / segmentSize;
		}

		CacheKey key(frameIndex, set);
		if (!m_Descriptors.contains(key))
			return CreateDescriptor(key, index, offset).get();

		const auto& indexVector = m_Descriptors[key];
		if (index >= indexVector.size())
			return CreateDescriptor(key, index, offset).get();

		const auto& offsetVector = indexVector[index];
		if (offset >= offsetVector.size())
			return CreateDescriptor(key, index, offset).get();

		const Ref<DescriptorSet>& pDescriptorSet = offsetVector[offset];
		if (!pDescriptorSet)
			return CreateDescriptor(key, index, offset).get();

		return pDescriptorSet.get();
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
}