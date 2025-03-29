#include "SceneRenderer.h"

#include "Poly/Model/Mesh.h"
#include "Poly/Model/Material.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/Sampler.h"
#include "Platform/API/DescriptorSet.h"
#include "Poly/Rendering/RenderScene.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Resources/ResourceManager.h"

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
		uint32 instanceCount = pSceneBatch->Matrices.size();

		Ref<Mesh> pMesh = pSceneBatch->MeshInstance.pMesh;
		const Buffer* pIndexBuffer = pMesh->GetIndexBuffer();
		commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

		commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
	}
}