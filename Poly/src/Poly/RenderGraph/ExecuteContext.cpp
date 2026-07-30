#include "ExecuteContext.h"

#include "Platform/API/CommandBuffer.h"
#include "Poly/RenderGraph/ResourceManager.h"

namespace Poly
{
	void ExecuteContext::SetTextureSlot(uint32 slot, const TextureHandle& textureHandle, const SamplerHandle& samplerHandle)
	{
		const SamplerHandle sampler   = samplerHandle.IsValid() ? samplerHandle : ResourceManager::GetDefaultLinearSampler();
		uint32              heapIndex = textureHandle.GetIndex() | (sampler.GetIndex() << ResourceManager::SAMPLER_INDEX_SHIFT);
		uint32              offset    = m_TextureSlotOffset + slot * static_cast<uint32>(sizeof(uint32));

		m_pCmdBuffer->UpdatePushConstants(m_pPipelineLayout, FShaderStage::VERTEX | FShaderStage::FRAGMENT, offset, sizeof(uint32), &heapIndex);
	}
} // namespace Poly