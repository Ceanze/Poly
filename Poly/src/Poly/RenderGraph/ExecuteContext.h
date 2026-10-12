#pragma once

#include "Poly/Core/Handle.h"
#include "RenderViewContext.h"

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
		               const RenderViewContext&    view,
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

		CommandBuffer* GetCommandBuffer() const { return m_pCmdBuffer; }

		/*
		 * Gets the view the pass is recorded for. Passes of a frame feature get the window as their view.
		 */
		const RenderViewContext& GetView() const { return m_View; }

		/*
		 * Gets the world of the view the pass is recorded for. Read-only, as passes may be recorded concurrently.
		 * @return The world, or nullptr for the passes of a frame feature.
		 */
		const World* GetWorld() const { return m_View.pWorld; }

	private:
		CommandBuffer*           m_pCmdBuffer;
		const RenderViewContext& m_View;
		PipelineLayout*          m_pPipelineLayout;
		uint32                   m_TextureSlotOffset;

		std::vector<DeclaredBuffer> m_DeclaredBuffers;
	};
} // namespace Poly
