#include "GeometryFeature.h"

#include "Platform/API/CommandBuffer.h"
#include "Poly/RenderGraph/ExecuteContext.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/World/Systems/RenderSystem.h"
#include "Poly/World/World.h"

namespace Poly
{
	// Scene resources (scene.*) are registered by the RenderSystem.
	// Until slang, the shader's bufferAddresses[] slots are assigned in MapGlobal() call order, so the order of the
	// MapGlobal() calls below is load bearing: Camera(0), scene.vertices(1), scene.instances(2), Lights(3), scene.materials(4).
	void GeometryFeature::Register(RenderCatalog& catalog)
	{
		catalog.RegisterResource(CAMERA_RESOURCE_NAME).WithType(EResourceType::UniformBuffer);
		catalog.RegisterResource(LIGHTS_RESOURCE_NAME).WithType(EResourceType::StorageBuffer);

		catalog.RegisterPass("pbr")
		    .WithShader("assets/shaders/pbr_bindless.vert")
		    .WithShader("assets/shaders/pbr_bindless.frag")
		    .MapResource(EFeaturePort::Color, "out_Color")
		    .MapResource(EFeaturePort::Depth, "depth")
		    .MapGlobal(CAMERA_RESOURCE_NAME, "camera")
		    .MapGlobal(RenderSystem::VERTICES_RESOURCE_NAME, "vertices")
		    .MapGlobal(RenderSystem::INSTANCE_RESOURCE_NAME, "instances")
		    .MapGlobal(LIGHTS_RESOURCE_NAME, "lights")
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
