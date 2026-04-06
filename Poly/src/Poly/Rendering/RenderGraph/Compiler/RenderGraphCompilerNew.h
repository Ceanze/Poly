#pragma once

#include "Poly/Rendering/RenderGraph/Compiler/RGCCompiler.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCDebugLogger.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCDebugTextureInjector.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCPassCompiler.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCProgramCreator.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceAllocator.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceOutputHandler.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceRegister.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCSynchroniser.h"
#include "Poly/Rendering/RenderGraph/Compiler/RGCValidator.h"
#include "Poly/Rendering/RenderGraph/RenderGraphTypes.h"

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
		RGCCompiler              m_GraphCompiler;
		RGCValidator             m_GraphValidator;
		RGCPassCompiler          m_GraphPassCompiler;
		RGCResourceRegister      m_GraphResourceRegister;
		RGCResourceOutputHandler m_GraphResourceOutputHandler;
		RGCResourceAllocator     m_GraphResourceAllocator;
		RGCSynchroniser          m_GraphSynchroniser;
		RGCDebugTextureInjector  m_GraphDebugTextureInjector;
		RGCDebugLogger           m_GraphDebugLogger;
		RGCProgramCreator        m_GraphProgramCreator;
	};
} // namespace Poly