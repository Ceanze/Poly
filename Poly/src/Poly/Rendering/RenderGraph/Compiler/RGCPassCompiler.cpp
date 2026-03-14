#include "Poly/Rendering/RenderGraph/Compiler/RGCPassCompiler.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/Pass.h"

namespace Poly
{
	void RGCPassCompiler::Execute(RGCContext& ctx)
	{
		for (auto& pass : ctx.CompiledGraph.CompiledPasses)
			pass.pPass->Compile();
	}
}