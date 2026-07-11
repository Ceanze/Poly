#pragma once

#include "RenderView.h"

namespace Poly
{
	class CommandBuffer;

	// Passed to PassDeclaration::WithExecuteFn() callbacks once per frame. By the time a pass's
	// ExecuteFn runs, RenderProgramInstance has already begun the render pass and bound the
	// pipeline, framebuffer, and descriptor sets for it - the callback only needs to issue draws
	// (and any per-frame buffer uploads) through the command buffer.
	class ExecuteContext
	{
	public:
		ExecuteContext(CommandBuffer* pCmdBuffer, const RenderView& view)
		    : m_pCmdBuffer(pCmdBuffer)
		    , m_View(view)
		{}

		CommandBuffer*    GetCommandBuffer() const { return m_pCmdBuffer; }
		const RenderView& GetView() const { return m_View; }

	private:
		CommandBuffer*    m_pCmdBuffer;
		const RenderView& m_View;
	};
} // namespace Poly
