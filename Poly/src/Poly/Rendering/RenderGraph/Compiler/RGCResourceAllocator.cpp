#include "Poly/Rendering/RenderGraph/Compiler/RGCResourceAllocator.h"

#include "Poly/Rendering/RenderGraph/Compiler/RGCContext.h"
#include "Poly/Rendering/RenderGraph/ResourceCache.h"

namespace Poly
{
	void RGCResourceAllocator::Execute(RGCContext& ctx)
	{
		if (!ctx.pResourceCache)
		{
			POLY_CORE_ERROR("No resource cache has been created/provided for the Render Graph Compiler");
			return;
		}

		ctx.pResourceCache->AllocateResources();
	}
}