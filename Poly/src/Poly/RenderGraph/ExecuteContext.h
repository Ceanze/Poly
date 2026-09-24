#pragma once

#include "Poly/Core/Handle.h"
#include "RenderView.h"

#include <string>
#include <string_view>
#include <utility>
#include <vector>

namespace Poly
{
	class Buffer;
	class CommandBuffer;
	class PipelineLayout;

	using TextureHandle = Handle<struct TextureHandleTag>;
	using SamplerHandle = Handle<struct SamplerHandleTag>;

	struct DeclaredBuffer
	{
		std::string Name; // global resource name, as passed to ReadResource/WriteResource
		Buffer*     pBuffer = nullptr;
	};

	class ExecuteContext
	{
	public:
		ExecuteContext(CommandBuffer*              pCmdBuffer,
		               const RenderView&           view,
		               PipelineLayout*             pPipelineLayout,
		               uint32                      textureSlotOffset,
		               std::vector<DeclaredBuffer> declaredBuffers = {})
		    : m_pCmdBuffer(pCmdBuffer)
		    , m_View(view)
		    , m_pPipelineLayout(pPipelineLayout)
		    , m_TextureSlotOffset(textureSlotOffset)
		    , m_DeclaredBuffers(std::move(declaredBuffers))
		{}

		void SetTextureSlot(uint32 slot, const TextureHandle& textureHandle, const SamplerHandle& samplerHandle);

		/*
		 * Gets a buffer the pass declared with ReadResource/WriteResource, e.g. an indirect argument buffer.
		 * @param resourceName The global name of the resource, as passed to ReadResource/WriteResource.
		 * @return The buffer, or nullptr if the pass did not declare the resource or it has not been supplied yet.
		 */
		Buffer* GetBuffer(std::string_view resourceName) const;

		CommandBuffer*    GetCommandBuffer() const { return m_pCmdBuffer; }
		const RenderView& GetView() const { return m_View; }

		/*
		 * Gets the world submitted for this frame. Read-only, as passes may be recorded concurrently.
		 * @return The world, or nullptr if no world was submitted for the window being rendered this frame.
		 */
		const World* GetWorld() const { return m_View.pWorld; }

	private:
		CommandBuffer*    m_pCmdBuffer;
		const RenderView& m_View;
		PipelineLayout*   m_pPipelineLayout;
		uint32            m_TextureSlotOffset;

		std::vector<DeclaredBuffer> m_DeclaredBuffers;
	};
} // namespace Poly
