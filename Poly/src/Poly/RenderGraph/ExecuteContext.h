#pragma once

#include "Poly/Core/Handle.h"
#include "RenderView.h"

namespace Poly
{
	class CommandBuffer;
	class PipelineLayout;

	using TextureHandle = Handle<struct TextureHandleTag>;
	using SamplerHandle = Handle<struct SamplerHandleTag>;

	class ExecuteContext
	{
	public:
		ExecuteContext(CommandBuffer* pCmdBuffer, const RenderView& view, PipelineLayout* pPipelineLayout, uint32 textureSlotOffset)
		    : m_pCmdBuffer(pCmdBuffer)
		    , m_View(view)
		    , m_pPipelineLayout(pPipelineLayout)
		    , m_TextureSlotOffset(textureSlotOffset)
		{}

		void SetTextureSlot(uint32 slot, const TextureHandle& textureHandle, const SamplerHandle& samplerHandle);

		CommandBuffer*    GetCommandBuffer() const { return m_pCmdBuffer; }
		const RenderView& GetView() const { return m_View; }

	private:
		CommandBuffer*    m_pCmdBuffer;
		const RenderView& m_View;
		PipelineLayout*   m_pPipelineLayout;
		uint32            m_TextureSlotOffset;
	};
} // namespace Poly
