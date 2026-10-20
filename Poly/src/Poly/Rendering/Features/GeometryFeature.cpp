#include "GeometryFeature.h"

#include "Platform/API/CommandBuffer.h"
#include "Poly/RenderGraph/ExecuteContext.h"
#include "Poly/RenderGraph/Feature/FeaturePort.h"
#include "Poly/RenderGraph/RenderCatalog.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/World/Systems/LightSystem.h"
#include "Poly/World/Systems/RenderSystem.h"
#include "Poly/World/World.h"

namespace
{
	void ExecuteSceneRaster(Poly::ExecuteContext& ctx)
	{
		const Poly::World*        pWorld        = ctx.GetWorld();
		const Poly::RenderSystem* pRenderSystem = pWorld ? pWorld->GetSystem<Poly::RenderSystem>() : nullptr;
		// An empty world has nothing to draw, and no geometry might have been loaded yet for the index buffer to exist
		if (!pRenderSystem || pRenderSystem->GetDrawBatches().empty())
			return;

		Poly::CommandBuffer* pCmd = ctx.GetCommandBuffer();
		pCmd->BindIndexBuffer(pRenderSystem->GetIndexBuffer(), 0, Poly::EIndexType::UINT32);
		for (const Poly::DrawBatch& batch : pRenderSystem->GetDrawBatches())
			pCmd->DrawIndexedInstanced(batch.IndexCount, batch.InstanceCount, batch.BaseIndex, batch.BaseVertex, batch.FirstInstance);
	}
} // namespace

namespace Poly
{
	void GeometryFeature::Register(RenderCatalog& catalog)
	{
		catalog.RegisterResource(Renderer::CAMERA_RESOURCE_NAME).WithType(EResourceType::UniformBuffer).WithScope(EResourceScope::View);
		catalog.RegisterResource(RenderSystem::VERTICES_RESOURCE_NAME).WithType(EResourceType::StorageBuffer).WithScope(EResourceScope::World);
		catalog.RegisterResource(RenderSystem::INSTANCE_RESOURCE_NAME).WithType(EResourceType::StorageBuffer).WithScope(EResourceScope::World);
		catalog.RegisterResource(RenderSystem::MATERIAL_RESOURCE_NAME).WithType(EResourceType::StorageBuffer).WithScope(EResourceScope::World);
		catalog.RegisterResource(LightSystem::LIGHTS_RESOURCE_NAME).WithType(EResourceType::StorageBuffer).WithScope(EResourceScope::World);

		catalog.GetExecutorRegistry().Register(SCENE_RASTER_EXECUTOR, "Draws the batches of the RenderSystem of the world being rendered", ExecuteSceneRaster);

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
		    .WithExecutor(SCENE_RASTER_EXECUTOR);

		catalog.RegisterFeature(FEATURE_NAME).WithPass("pbr");
	}
} // namespace Poly
