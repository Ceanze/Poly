#pragma once

#include "PassData.h"
#include "RenderGraphTypes.h"
#include "Reflection/PassReflection.h"

namespace Poly
{
	class Pass;
	class Resource;
	class RenderPass;
	class RenderGraph;
	class ResourceGUID;
	class ResourceCache;
	class RenderGraphProgram;

	class RenderGraphCompiler
	{
	public:
		RenderGraphCompiler() = default;
		~RenderGraphCompiler() = default;

		static Ref<RenderGraphCompiler> Create();

		Ref<RenderGraphProgram> Compile(RenderGraph* pRenderGraph, RenderGraphDefaultParams defaultParams);

	private:
		void SetupExecutionOrder();
		void CompilePasses();
		void ValidateGraph();
		void AllocateResources();
		void AddSync(bool* pNewPasses);
		bool IsResourceUsed(const ResourceGUID& resourceGUID, uint32 nodeIndex);
		bool IsResourceGraphOutput(const ResourceGUID& resourceGUID, uint32 nodeIndex);
		FAccessFlag GetAccessFlag(FResourceBindPoint bindPoint, bool isInput);
		FPipelineStage GetPipelineStage(FResourceBindPoint bindPoint);

		RenderGraph*				m_pRenderGraph = nullptr;
		std::vector<PassData>		m_OrderedPasses;
		Ref<ResourceCache>			m_pResourceCache = nullptr;
		RenderGraphDefaultParams	m_DefaultParams = {};
	};
}