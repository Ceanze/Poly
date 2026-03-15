#include "polypch.h"
#include "RGCProgramCreator.h"

#include "Poly/Rendering/RenderGraph/PassData.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"

namespace Poly
{
	Ref<RenderGraphProgram> RGCProgramCreator::Execute(RGCContext& ctx)
	{
		std::vector<PassData> passes;
		passes.reserve(ctx.CompiledGraph.CompiledPasses.size());

		for (uint32 i = 0; i < ctx.CompiledGraph.CompiledPasses.size(); i++)
		{
			const CompiledPass& compiledPass = ctx.CompiledGraph.CompiledPasses[i];

			PassData passData		= {};
			passData.pPass			= compiledPass.pPass;
			passData.Reflection		= compiledPass.Reflection;
			passData.NodeIndex		= compiledPass.GraphNodeIndex;
			passData.PassIndex		= i;

			passes.push_back(std::move(passData));
		}

		Ref<RenderGraphProgram> pProgram = RenderGraphProgram::Create(ctx.pResourceCache, ctx.DefaultParams, std::move(passes));
		pProgram->SetDebugTextureGUIDs(ctx.DebugTextureGUIDs);
		return pProgram;
	}
}
