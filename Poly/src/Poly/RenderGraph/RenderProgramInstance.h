#pragma once

#include "Poly/Core/Core.h"
#include "RenderProgram.h"

namespace Poly
{
	struct RenderView;

	/*
	 * An instantiated, active version of a RenderProgram. Holds the compiled RenderProgram
	 * together with the runtime graphics resources allocated for it.
	 *
	 * One instance exists per window (see Renderer::WindowContext) so that each window can
	 * render a different view (camera/scene/target) and keep its own frame-in-flight state,
	 * while all instances may share the same compiled RenderProgram.
	 *
	 * Ownership: constructed by Renderer for each window once a RenderProgram becomes active,
	 * at a point where it's safe to retire the previously active instance (see Renderer).
	 */
	class RenderProgramInstance
	{
	public:
		static constexpr uint32 FRAMES_IN_FLIGHT = 2;

		explicit RenderProgramInstance(Ref<RenderProgram> pRenderProgram);
		~RenderProgramInstance() = default;
		CLASS_REMOVE_COPY(RenderProgramInstance);

		void Execute(const RenderView& view);

		const RenderProgram& GetProgram() const { return *m_pRenderProgram; }

	private:
		Ref<RenderProgram> m_pRenderProgram;
		uint32             m_FrameIndex = 0;
		// TODO (future): runtime handles for allocated GPU resources, pipeline/framebuffer caches
	};
} // namespace Poly
