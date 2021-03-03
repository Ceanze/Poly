#pragma once

#include "Poly/Core/Core.h"
#include "RenderPassReflection.h"

namespace Poly
{
	class Resource;
	class RenderPass;
	class RenderGraph;
	class ResourceCache;

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
		void ValidateGraph();
		void AllocateResources();
		void AddSync();
		bool IsResourceUsed(uint32 nodeIndex, const std::string& outputName);

		RenderGraph* m_pRenderGraph = nullptr;
		std::vector<PassData> m_OrderedPasses;
		Ref<ResourceCache> m_pResourceCache;
	};
}