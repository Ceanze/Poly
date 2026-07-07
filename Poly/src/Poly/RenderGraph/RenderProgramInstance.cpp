#include "RenderProgramInstance.h"

#include "RenderView.h"

namespace Poly
{
	RenderProgramInstance::RenderProgramInstance(std::unique_ptr<RenderProgram> pRenderProgram)
	    : m_pRenderProgram(std::move(pRenderProgram))
	{}

	void RenderProgramInstance::Execute(const RenderView& view)
	{
		// TODO: iterate m_pRenderProgram->GetPasses() and dispatch pass execution once
		// SetupContext/ExecuteContext and GPU resource allocation exist.
		(void)view;
	}
} // namespace Poly
