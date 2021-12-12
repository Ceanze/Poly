#pragma once

#include "PassReflection.h"
#include "RenderGraphTypes.h"

namespace Poly
{
	class Pass;
	class Resource;
	class RenderPass;
	class RenderGraph;
	class ResourceCache;
	class RenderGraphProgram;

	class RenderGraphCompiler
	{
	private:
		struct PassData
		{
			Ref<Pass>		pPass;
			uint32			NodeIndex;
			PassReflection	Reflection;

			bool operator== (const PassData& other) const { return NodeIndex == other.NodeIndex; }
		};

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
		bool IsResourceUsed(uint32 nodeIndex, const std::string& outputName);
		bool IsResourceGraphOutput(uint32 nodeIndex, const std::string& outputName);
		FAccessFlag GetAccessFlag(FResourceBindPoint bindPoint, bool isInput);
		FPipelineStage GetPipelineStage(FResourceBindPoint bindPoint);

		RenderGraph*				m_pRenderGraph = nullptr;
		std::vector<PassData>		m_OrderedPasses;
		Ref<ResourceCache>			m_pResourceCache = nullptr;
		RenderGraphDefaultParams	m_DefaultParams = {};
	};
}