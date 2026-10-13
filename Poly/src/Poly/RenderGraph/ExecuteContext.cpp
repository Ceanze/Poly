#include "ExecuteContext.h"

#include "Platform/API/CommandBuffer.h"
#include "Poly/RenderGraph/ResourceManager.h"

namespace Poly
{
	void ExecuteContext::SetTexture(std::string_view shaderName, const TextureHandle& textureHandle, const SamplerHandle& samplerHandle)
	{
		auto it = std::ranges::find(m_TextureFields, shaderName, &TextureField::ShaderName);
		if (it == m_TextureFields.end())
		{
			POLY_CORE_ERROR("The shaders of this pass have no TextureHandle named '{}' in their push constants", shaderName);
			return;
		}

		const SamplerHandle sampler   = samplerHandle.IsValid() ? samplerHandle : ResourceManager::GetDefaultLinearSampler();
		uint32              heapIndex = textureHandle.GetIndex() | (sampler.GetIndex() << ResourceManager::SAMPLER_INDEX_SHIFT);

		m_pCmdBuffer->UpdatePushConstants(m_pPipelineLayout, FShaderStage::VERTEX | FShaderStage::FRAGMENT, it->Offset, sizeof(uint32), &heapIndex);
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