#include "GeometryFeature.h"

#include "Platform/API/CommandBuffer.h"
#include "Poly/RenderGraph/ExecuteContext.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/World/Systems/LightSystem.h"
#include "Poly/World/Systems/RenderSystem.h"
#include "Poly/World/World.h"

namespace Poly
{
	// Scene resources (scene.*) are registered by the RenderSystem and the LightSystem.
	// The shader names passed to MapGlobal() are the names of the pointers in the push constants of pbr_bindless.slang.
	void GeometryFeature::Register(RenderCatalog& catalog)
	{
		catalog.RegisterResource(Renderer::CAMERA_RESOURCE_NAME).WithType(EResourceType::UniformBuffer).WithScope(EResourceScope::View);

		catalog.RegisterPass("pbr")
		    .WithShader("assets/shaders/pbr_bindless.slang", FShaderStage::VERTEX)
		    .WithShader("assets/shaders/pbr_bindless.slang", FShaderStage::FRAGMENT)
		    .MapResource(EFeaturePort::Color, "out_Color")
		    .MapResource(EFeaturePort::Depth, "depth")
		    .MapGlobal(Renderer::CAMERA_RESOURCE_NAME, "camera")
		    .MapGlobal(RenderSystem::VERTICES_RESOURCE_NAME, "vertices")
		    .MapGlobal(RenderSystem::INSTANCE_RESOURCE_NAME, "instances")
		    .MapGlobal(LightSystem::LIGHTS_RESOURCE_NAME, "lights")
		    .MapGlobal(RenderSystem::MATERIAL_RESOURCE_NAME, "materialProps")
		    .WithExecuteFn([](ExecuteContext& ctx) {
			    const World*        pWorld        = ctx.GetWorld();
			    const RenderSystem* pRenderSystem = pWorld ? pWorld->GetSystem<RenderSystem>() : nullptr;
			    // An empty world has nothing to draw, and no geometry might have been loaded yet for the index buffer to exist
			    if (!pRenderSystem || pRenderSystem->GetDrawBatches().empty())
				    return;

			    CommandBuffer* pCmd = ctx.GetCommandBuffer();
			    pCmd->BindIndexBuffer(pRenderSystem->GetIndexBuffer(), 0, EIndexType::UINT32);
			    for (const DrawBatch& batch : pRenderSystem->GetDrawBatches())
				    pCmd->DrawIndexedInstanced(batch.IndexCount, batch.InstanceCount, batch.BaseIndex, batch.BaseVertex, batch.FirstInstance);
		    });

		catalog.RegisterFeature(FEATURE_NAME).WithPass("pbr");
	}
} // namespace Poly
