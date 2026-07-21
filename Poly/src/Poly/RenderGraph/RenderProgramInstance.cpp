#include "RenderProgramInstance.h"

#include "RenderView.h"

namespace Poly
{
	RenderProgramInstance::RenderProgramInstance(std::unique_ptr<RenderProgram> pRenderProgram)
	    : m_pRenderProgram(std::move(pRenderProgram))
	{}

	void RenderProgramInstance::Execute(const RenderView& view)
	{
		(void)view;

		m_FrameIndex = (m_FrameIndex + 1) % FRAMES_IN_FLIGHT;
	}
} // namespace Poly
