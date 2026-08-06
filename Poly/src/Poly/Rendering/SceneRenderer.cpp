#include "SceneRenderer.h"

#include "Platform/API/Buffer.h"
#include "Poly/Model/Mesh.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"
#include "Poly/Rendering/RenderScene.h"

namespace Poly
{
	void SceneRenderer::Execute(const RenderContext& context)
	{
		Render(context);
	}

	void SceneRenderer::Render(const RenderContext& context)
	{
		CommandBuffer* commandBuffer = context.GetCommandBuffer();

		const SceneBatch* pSceneBatch   = context.GetSceneBatch();
		uint32            instanceCount = static_cast<uint32>(pSceneBatch->Matrices.size());

		Ref<Mesh>     pMesh        = pSceneBatch->MeshInstance.pMesh;
		const Buffer* pIndexBuffer = nullptr; // pMesh->GetIndexBuffer(); NOTE: Deprecated for RG2
		commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

		// commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0); NOTE: Deprecated for RG2
		commandBuffer->DrawIndexedInstanced(0, instanceCount, 0, 0, 0);
	}
} // namespace Poly