#pragma once

#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/Compiler/CompiledGraph.h"

namespace Poly
{
	class ResourceCache;

	struct RGCContext
	{
		RGCContext(RenderGraph renderGraph, RenderGraphDefaultParams defaultParams)
			: RenderGraph(std::move(renderGraph))
			, DefaultParams(std::move(defaultParams)) {}

		RenderGraph RenderGraph;
		RenderGraphDefaultParams DefaultParams;
		Ref<ResourceCache> pResourceCache;
		CompiledGraph CompiledGraph;
		bool IsGraphDirty = false;;
	};
}