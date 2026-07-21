#pragma once

#include "Poly/Core/Core.h"
#include "RenderProgram.h"

#include <memory>

namespace Poly
{
	struct RenderView;

	/*
	 * An instantiated, active version of a RenderProgram. Holds the compiled RenderProgram
	 * together with the runtime graphics resources allocated for it.
	 *
	 * Ownership: constructed by Renderer::SetRenderProgram() and moved into place once it
	 * becomes safe to swap out the previously active instance (see Renderer).
	 */
	class RenderProgramInstance
	{
	public:
		static constexpr uint32 FRAMES_IN_FLIGHT = 2;

		explicit RenderProgramInstance(std::unique_ptr<RenderProgram> pRenderProgram);
		~RenderProgramInstance() = default;
		CLASS_REMOVE_COPY(RenderProgramInstance);

		void Execute(const RenderView& view);

		const RenderProgram& GetProgram() const { return *m_pRenderProgram; }

	private:
		std::unique_ptr<RenderProgram> m_pRenderProgram;
		uint32                         m_FrameIndex = 0;
		// TODO (future): runtime handles for allocated GPU resources, pipeline/framebuffer caches
	};
} // namespace Poly
