#include "Poly/Rendering/RenderGraph/Compiler/RenderGraphCompilerNew.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"

namespace Poly
{
	RenderGraphCompilerNew::RenderGraphCompilerNew() {}

	Ref<RenderGraphProgram> RenderGraphCompilerNew::Compile(RenderGraph* pRenderGraph, RenderGraphDefaultParams defaultParams)
	{
		RGCContext ctx(pRenderGraph->Clone(), defaultParams);

		ctx.pResourceCache = ResourceCache::Create(defaultParams);

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

		// TODO: Add a RGCResourceLifetimeCalculator to handle the lifetime setting of the resource cache (ignored for now)

		return m_GraphProgramCreator.Execute(ctx);
	}
}