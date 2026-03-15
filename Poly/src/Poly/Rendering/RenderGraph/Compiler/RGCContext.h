#pragma once

#include "RGCSyncTypes.h"

#include "Poly/Rendering/RenderGraph/RenderGraph.h"
#include "Poly/Rendering/RenderGraph/Compiler/CompiledGraph.h"
#include "Poly/Rendering/RenderGraph/ResourceGUID.h"

#include <unordered_map>
#include <vector>

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
		bool IsGraphDirty = false;

		// Written by RGCSynchroniser — final layout/access/stage of every physical
		// resource after all normal sync passes have been inserted.
		// Key: canonical ResourceGUID (ResourceCache::GetCanonicalGUID).
		std::unordered_map<ResourceGUID, ResourceState, ResourceGUIDHasher> PostSyncResourceStates;

		// Written by RGCDebugTextureInjector — GUIDs of texture outputs made
		// available for ImGui::Image() sampling.  Transferred to RenderGraphProgram.
		std::vector<ResourceGUID> DebugTextureGUIDs;
	};
}