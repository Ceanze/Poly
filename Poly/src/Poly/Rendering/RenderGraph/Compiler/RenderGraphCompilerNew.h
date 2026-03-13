#pragma once

#include "Poly/Rendering/RenderGraph/RenderGraphTypes.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCCompiler.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCValidator.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCPassCompiler.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceRegister.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceOutputHandler.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceAllocator.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCSynchroniser.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCProgramCreator.h"

namespace Poly
{
	class RenderGraph;
	class RenderGraphProgram;

	class RenderGraphCompilerNew
	{
	public:
		RenderGraphCompilerNew();
		~RenderGraphCompilerNew() = default;

		Ref<RenderGraphProgram> Compile(RenderGraph* pRenderGraph, RenderGraphDefaultParams defaultParams);

	private:
		RGCCompiler m_GraphCompiler;
		RGCValidator m_GraphValidator;
		RGCPassCompiler m_GraphPassCompiler;
		RGCResourceRegister m_GraphResourceRegister;
		RGCResourceOutputHandler m_GraphResourceOutputHandler;
		RGCResourceAllocator m_GraphResourceAllocator;
		RGCSynchroniser		m_GraphSynchroniser;
		RGCProgramCreator	m_GraphProgramCreator;
	};
}

/*
Steps:
1. Setup GraphContext - this is sent to every step. Copy the render graph so that we can safely mutate it (for adding sync passes)
2. Analyse Graph - Generates a CompiledGraph output, containing the execution order based on mandatory passes, with reflection and metadata for passes
	2a. Reflect passes - Use a cache for this to avoid previously added passes to be reflected again (in the case of when we add sync and call this again)
3. Validate Graph - Validate the generated graph is good to continue
4. Compile Passes - Goes through and calls Compile on each passes (nothing more for now)
5. Alias Resources - Sets up the aliases of resources. Does not allocate, but generates a resource cache
6. Mark Output - Marks the output/s that are outputs as such. Do not consider multiple or several render graphs atm.
7. Allocate Resources - Allocates the resources based on aliases and outputs. Needs to be separated to handle multiple graphs (future project). This can be skipped or done later as well.
8. Sync Graph - Adds synchronisation steps to handle WAR, RAW, and WAW as well as layout changes, and queue changes
9. Analyse and Validate after sync additions - Repeats step 2 and 3 with the manipulated graph to finalise execution order
10. Create Render Graph Program
*/

/*
✅ 1. CreateGraphContext(...) -> GraphContext{RenderGraph*, DefaultParams, ResourceCache*, CompiledGraph}
✅ 2. m_GraphCompiler.Execute(ctx) -> Modifies CompiledGraph [SetupExecutionOrder]
✅ 3. m_GraphValidator.Execute(ctx) -> returns Valid, no modification [ValidateGraph]
✅ 4. m_GraphPassCompiler.Execute(ctx) -> Calls CompilePass, no modification [CompilePasses]
✅ 5. m_GraphResourceRegister.Execute(ctx) -> Modifies ResourceCache [AllocateResources, just register and alias]
✅ 6. m_GraphResourceOutputHandler.Execute(ctx) -> Modifies ResourceCache [AllocateResources, MarkOutput call/s]
✅ 7. m_GraphResourceAllocator.Execute(ctx) -> Modifies ResoruceCache [AllocateResources, AllocateResource call]
8. m_GraphSynchroniser.Execute(ctx) -> Modifies RenderGraph (new passes), ResourceCache (aliases) -> Returns true if changed [AddSync]
9. if (changed) {m_GraphCompiler.Execute(ctx); m_GraphValidator(ctx);}
10. m_GraphResourceLifetimeCalculator(ctx) -> Modifies ResourceCache [Sets the Timepoint variables in the cache]
11. m_GraphProgramCreator.Execute(ctx) -> Returns a created RenderGraphProgram [RenderGraphProgram::Create call]
*/