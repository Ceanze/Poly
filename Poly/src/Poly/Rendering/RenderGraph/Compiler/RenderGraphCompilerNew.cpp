#include "Poly/Rendering/RenderGraph/Compiler/RenderGraphCompilerNew.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"

namespace Poly
{
	RenderGraphCompiler::RenderGraphCompiler() {}

	Ref<RenderGraphProgram> RenderGraphCompiler::Compile(RenderGraph* pRenderGraph, RenderGraphDefaultParams defaultParams)
	{
		RGCContext ctx(pRenderGraph->Clone(), defaultParams);

		m_GraphCompiler.Execute(ctx);
		if (!m_GraphValidator.Execute(ctx))
			return nullptr;
		m_GraphPassCompiler.Execute(ctx);
		m_GraphResourceRegister.Execute(ctx);
		m_GraphResourceOutputHandler.Execute(ctx);
		m_GraphResourceAllocator.Execute(ctx);
		m_GraphSynchroniser.Execute(ctx);
		if (ctx.IsGraphDirty)
		{
			ctx.IsGraphDirty = false;
			m_GraphCompiler.Execute(ctx);
			if (!m_GraphValidator.Execute(ctx))
				return nullptr;
		}

		return nullptr;
	}
}