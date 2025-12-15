#include "DescriptorCache.h"

#include "Poly/Core/RenderAPI.h"
#include "Platform/API/DescriptorSet.h"

namespace Poly
{
	DescriptorCache::DescriptorCache()
	{
		m_pSyncPoint = RenderAPI::CreateSyncPoint();
	}

	SyncPointValue DescriptorCache::GetSignalSyncPointValue()
	{
		return { m_pSyncPoint.get(), m_SyncPointValue };
	}

	void DescriptorCache::Update()
	{
		m_SyncPointValue++;
		const uint64 currentSignaledValue = m_pSyncPoint->GetValue();

		auto it = std::remove_if(m_RemovableDescriptors.begin(), m_RemovableDescriptors.end(), [currentSignaledValue](const RemovableDescriptorSet& info) { return info.SyncPointValue < currentSignaledValue; });
		m_RemovableDescriptors.erase(it, m_RemovableDescriptors.end());
	}

	DescriptorSet* DescriptorCache::GetDescriptorSetCopy(uint32 set, uint32 index, uint32 offset, uint32 segmentSize)
	{
		if (!ValidateOffset(offset, segmentSize))
			return nullptr;

		if (!HasDescriptor(set, index, offset))
			return CreateDescriptor(set, index, offset).get();

		Ref<DescriptorSet> pOldDescriptor = m_Descriptors[set][index][offset];

		Ref<DescriptorSet> pNewDescriptor = RenderAPI::CreateDescriptorSetCopy(pOldDescriptor);
		m_Descriptors[set][index][offset] = pNewDescriptor;

		m_RemovableDescriptors.push_back(RemovableDescriptorSet(pOldDescriptor, m_SyncPointValue));

		return pNewDescriptor.get();
	}

	void DescriptorCache::SetPipelineLayout(PipelineLayout* pPipelineLayout)
	{
		m_pPipelineLayout = pPipelineLayout;
	}

	DescriptorSet* DescriptorCache::GetDescriptorSet(uint32 set, EAction action)
	{
		return GetDescriptorSet(set, 0, 0, 0, action);
	}

	DescriptorSet* DescriptorCache::GetDescriptorSet(uint32 set, uint32 index, EAction action)
	{
		return GetDescriptorSet(set, index, 0, 0, action);
	}

	DescriptorSet* DescriptorCache::GetDescriptorSet(uint32 set, uint32 index, uint32 offset, uint32 segmentSize, EAction action)
	{
		if (!ValidateOffset(offset, segmentSize))
			return nullptr;

		if (!HasDescriptor(set, index, offset)) {
			switch (action)
			{
			case Poly::DescriptorCache::EAction::GET:
				return nullptr;
			case Poly::DescriptorCache::EAction::GET_OR_CREATE:
				return CreateDescriptor(set, index, offset).get();
			}
		}

		return m_Descriptors[set][index][offset].get();
	}

	Ref<DescriptorSet> DescriptorCache::CreateDescriptor(uint32 set, uint32 index, uint32 offset)
	{
		Ref<DescriptorSet> pDescriptorSet = RenderAPI::CreateDescriptorSet(m_pPipelineLayout, set);

		auto& indexVector = m_Descriptors[set];
		if (index + 1 >= static_cast<uint32>(indexVector.size()))
			indexVector.resize(index + 1);

		auto& offsetVector = indexVector[index];
		if (offset + 1 >= static_cast<uint32>(offsetVector.size()))
			offsetVector.resize(offset + 1);

		offsetVector[offset] = pDescriptorSet;
		return pDescriptorSet;
	}

	bool DescriptorCache::HasDescriptor(uint32 set, uint32 index, uint32 offset)
	{
		if (!m_Descriptors.contains(set))
			return false;

		const auto& indexVector = m_Descriptors[set];
		if (index >= static_cast<uint32>(indexVector.size()))
			return false;

		const auto& offsetVector = indexVector[index];
		if (offset >= static_cast<uint32>(offsetVector.size()))
			return false;

		const Ref<DescriptorSet>& pDescriptorSet = offsetVector[offset];
		if (!pDescriptorSet)
			return false;

		return true;
	}

	bool DescriptorCache::ValidateOffset(uint32& offset, uint32 segmentSize)
	{
		if (offset > 0)
		{
			if (segmentSize == 0)
			{
				POLY_CORE_ERROR("Tried to get descriptor with offset {}, but segment size was zero. If offset is used then segment size must be larger than zero.",
								offset);
				return false;
			}
			offset = offset / segmentSize;
		}

		return true;
	}
}