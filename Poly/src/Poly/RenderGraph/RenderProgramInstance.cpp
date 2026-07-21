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
	}
} // namespace Poly
