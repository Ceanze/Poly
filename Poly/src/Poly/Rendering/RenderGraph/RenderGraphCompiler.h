#pragma once

#include "Poly/Core/Core.h"

namespace Poly
{
	class RenderGraph;
	class Resource;

	class RenderGraphCompiler
	{
	private:
		struct PassData
		{
			Ref<Resource>	pResource;
			uint32			NodeIndex;
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