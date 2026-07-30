#pragma once

#include "Poly/Core/Core.h"
#include "Poly/Model/Material.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/RenderGraph/Shader/GPUInstanceData.h"
#include "Poly/RenderGraph/Shader/GPUMaterialData.h"
#include "Poly/Rendering/Core/API/GraphicsTypes.h"

#include <glm/glm.hpp>

#include <array>
#include <cstddef>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Poly
{
	class Scene;
	class Mesh;
	class Material;
	class Buffer;
	class CommandPool;
	class CommandBuffer;
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
	// TODO: Handle vertex and index buffer combination at load time to avoid double storage of mesh data in GPU memory.
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
	 * Rebuild cost: every Update() that finds the scene dirty fully rebuilds all four buffers (no
	 * incremental patching). Mesh/index data is copied GPU->GPU straight from each mesh's own buffer
	 * (no CPU readback - meshes don't keep a CPU-side copy after upload).
	 */
	class SceneRenderBridge
	{
	public:
		SceneRenderBridge(Scene& scene, Ref<RenderProgramInstance> pProgramInstance);
		~SceneRenderBridge() = default;
		CLASS_REMOVE_COPY(SceneRenderBridge);

		void Update();

		const std::vector<SceneDrawBatch>& GetDrawBatches() const { return m_DrawBatches; }
		Buffer*                            GetIndexBuffer() const { return ResourceManager::Resolve(m_IndexBufferHandle); }

	private:
		struct MeshRange
		{
			uint32 BaseVertex;
			uint32 BaseIndex;
			uint32 IndexCount;
		};

		GPUMaterialData BuildMaterialData(Material* pMaterial);
		void            RebuildCombinedMeshBuffers(const std::vector<std::pair<Mesh*, MeshRange>>& meshesToCopy, uint32 totalVertices, uint32 totalIndices);
		void            UploadInstanceAndMaterialBuffers(const std::vector<GPUInstanceData>& instances, const std::vector<GPUMaterialData>& materials);

		Scene&                     m_Scene;
		Ref<RenderProgramInstance> m_pProgramInstance;

		// Must be on the same queue as the meshes (graphics queue) to avoid queue ownership transfer
		Ref<CommandPool> m_pCopyCommandPool;
		CommandBuffer*   m_pCopyCommandBuffer = nullptr;

		std::unordered_map<Material*, std::array<uint32, 6>> m_MaterialTextureCache;

		std::vector<SceneDrawBatch> m_DrawBatches;

		BufferHandle m_VertexBufferHandle;
		BufferHandle m_IndexBufferHandle;
		BufferHandle m_InstanceBufferHandle;
		BufferHandle m_MaterialBufferHandle;
	};
} // namespace Poly
