#pragma once

#include "Poly/Core/Core.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/RenderGraph/Shader/GPUInstanceData.h"
#include "Poly/RenderGraph/Shader/GPUMaterialData.h"

#include <array>
#include <unordered_map>
#include <vector>

namespace Poly
{
	class Scene;
	class Material;
	class Buffer;
	class RenderProgramInstance;

	struct SceneDrawBatch
	{
		uint32 BaseVertex    = 0;
		uint32 BaseIndex     = 0;
		uint32 IndexCount    = 0;
		uint32 FirstInstance = 0;
		uint32 InstanceCount = 0;
	};

	// TODO: Rename to RenderScene when old RenderScene is deprecated
	/*
	 * Entities sharing the same (Mesh, Material) pair are batched into one instanced draw call.
	 * A pass would consume this as:
	 *
	 *   .WithExecuteFn([pBridge](ExecuteContext& ctx) {
	 *       CommandBuffer* pCmd = ctx.GetCommandBuffer();
	 *       pCmd->BindIndexBuffer(pBridge->GetIndexBuffer(), 0, EIndexType::UINT32);
	 *       for (const SceneDrawBatch& b : pBridge->GetDrawBatches())
	 *           pCmd->DrawIndexedInstanced(b.IndexCount, b.InstanceCount, b.BaseIndex, b.BaseVertex, b.FirstInstance);
	 *   });
	 *
	 */
	class SceneRenderBridge
	{
	public:
		SceneRenderBridge(Scene& scene, Ref<RenderProgramInstance> pProgramInstance);
		~SceneRenderBridge() = default;
		CLASS_REMOVE_COPY(SceneRenderBridge);

		void Update();

		const std::vector<SceneDrawBatch>& GetDrawBatches() const { return m_DrawBatches; }
		Buffer*                            GetIndexBuffer() const;

	private:
		struct MeshRange
		{
			uint32 BaseVertex;
			uint32 BaseIndex;
			uint32 IndexCount;
		};

		GPUMaterialData BuildMaterialData(Material* pMaterial);
		void            UploadInstanceAndMaterialBuffers(const std::vector<GPUInstanceData>& instances, const std::vector<GPUMaterialData>& materials);

		Scene&                     m_Scene;
		Ref<RenderProgramInstance> m_pProgramInstance;

		std::unordered_map<Material*, std::array<uint32, 6>> m_MaterialTextureCache;

		std::vector<SceneDrawBatch> m_DrawBatches;

		BufferHandle m_InstanceBufferHandle;
		BufferHandle m_MaterialBufferHandle;
	};
} // namespace Poly
