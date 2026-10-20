#pragma once

#include "Poly/Core/Core.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/RenderGraph/Shader/GPUInstanceData.h"
#include "Poly/RenderGraph/Shader/GPUMaterialData.h"
#include "Poly/Resources/AssetHandle.h"

#include <array>
#include <unordered_map>
#include <vector>

namespace Poly
{
	class World;
	class Buffer;
	class MaterialAsset;
	class TextureAsset;

	struct DrawBatch
	{
		uint32 BaseVertex    = 0;
		uint32 BaseIndex     = 0;
		uint32 IndexCount    = 0;
		uint32 FirstInstance = 0;
		uint32 InstanceCount = 0;
	};

	/*
	 * Builds the GPU-side scene data (vertices, instances, materials) for all renderable entities of a world
	 * and provides it to the render graph through the world's RenderResourceTable.
	 * Entities sharing the same (Mesh, Material) pair are batched into one instanced draw call.
	 *
	 * Usage:
	 *
	 *   world.AddSystem<RenderSystem>(World::Phase::PostUpdate);
	 *
	 *   pass.MapGlobal(RenderSystem::VERTICES_RESOURCE_NAME, "vertices")
	 *       .MapGlobal(RenderSystem::INSTANCE_RESOURCE_NAME, "instances")
	 *       .MapGlobal(RenderSystem::MATERIAL_RESOURCE_NAME, "materialProps")
	 *       .WithExecuteFn([](ExecuteContext& ctx) {
	 *           const World*        pWorld  = ctx.GetWorld();
	 *           const RenderSystem* pSystem = pWorld ? pWorld->GetSystem<RenderSystem>() : nullptr;
	 *           if (!pSystem)
	 *               return;
	 *
	 *           CommandBuffer* pCmd = ctx.GetCommandBuffer();
	 *           pCmd->BindIndexBuffer(pSystem->GetIndexBuffer(), 0, EIndexType::UINT32);
	 *           for (const DrawBatch& b : pSystem->GetDrawBatches())
	 *               pCmd->DrawIndexedInstanced(b.IndexCount, b.InstanceCount, b.BaseIndex, b.BaseVertex, b.FirstInstance);
	 *       });
	 */
	class RenderSystem
	{
	public:
		static constexpr const char* VERTICES_RESOURCE_NAME = "scene.vertices";
		static constexpr const char* INSTANCE_RESOURCE_NAME = "scene.instances";
		static constexpr const char* MATERIAL_RESOURCE_NAME = "scene.materials";

		RenderSystem()  = default;
		~RenderSystem() = default;
		CLASS_REMOVE_COPY(RenderSystem);

		void Update(World& world);
		void OnShutdown(World& world);

		const std::vector<DrawBatch>& GetDrawBatches() const { return m_DrawBatches; }
		Buffer*                       GetIndexBuffer() const;

	private:
		void            Rebuild(World& world);
		GPUMaterialData BuildMaterialData(AssetHandle<MaterialAsset> materialHandle);
		uint32          PackTextureIndex(AssetHandle<TextureAsset> textureHandle);
		void            UploadInstanceAndMaterialBuffers(const std::vector<GPUInstanceData>& instances, const std::vector<GPUMaterialData>& materials);

		// Forces a rebuild on the first update, even if the world has no dirty entities by the time the system is added
		bool m_NeedsRebuild = true;

		std::unordered_map<AssetHandle<MaterialAsset>, std::array<uint32, 6>> m_MaterialTextureCache;

		std::vector<DrawBatch> m_DrawBatches;

		BufferHandle m_InstanceBufferHandle;
		BufferHandle m_MaterialBufferHandle;
	};
} // namespace Poly
