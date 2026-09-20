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

	Buffer* ExecuteContext::GetBuffer(std::string_view resourceName) const
	{
		for (const DeclaredBuffer& declared : m_DeclaredBuffers)
		{
			if (declared.Name == resourceName)
				return declared.pBuffer;
		}

		POLY_CORE_ERROR("Resource '{}' was not declared by this pass via ReadResource/WriteResource, or has not been supplied yet", resourceName);
		return nullptr;
	}
} // namespace Poly