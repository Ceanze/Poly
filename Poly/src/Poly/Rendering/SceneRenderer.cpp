#include "SceneRenderer.h"

#include "Poly/Model/Mesh.h"
#include "Platform/API/Buffer.h"
#include "Poly/Rendering/RenderScene.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"

namespace Poly
{
	void SceneRenderer::Execute(const RenderContext& context)
	{
		Render(context);
	}

	void SceneRenderer::Render(const RenderContext& context)
	{
		CommandBuffer* commandBuffer = context.GetCommandBuffer();

		const SceneBatch* pSceneBatch = context.GetSceneBatch();
		uint32 instanceCount = static_cast<uint32>(pSceneBatch->Matrices.size());

		Ref<Mesh> pMesh = pSceneBatch->MeshInstance.pMesh;
		const Buffer* pIndexBuffer = pMesh->GetIndexBuffer();
		commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

		commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
	}
}