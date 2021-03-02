#pragma once

#include "Poly/Core/Core.h"
#include "RenderPassReflection.h"

namespace Poly
{
	class Resource;
	class RenderPass;
	class RenderGraph;

	class RenderGraphCompiler
	{
	private:
		struct PassData
		{
			Ref<RenderPass>			pPass;
			uint32					NodeIndex;
			RenderPassReflection	Reflection;
		};

	public:
		RenderGraphCompiler() = default;
		~RenderGraphCompiler() = default;

		static Ref<RenderGraphCompiler> Create();

		void Compile(RenderGraph* pRenderGraph);

	private:
		void SetupExecutionOrder();
		void CompilePasses();
		void AddSync();
		void ValidateGraph();
		void AllocateResources();

		RenderGraph* m_pRenderGraph = nullptr;
		std::vector<PassData> m_OrderedPasses;
	};
}