#include "SceneRenderer.h"

#include "Poly/Model/Mesh.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/DescriptorSet.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"

namespace Poly
{
	void SceneRenderer::Update(uint32 setIndex, uint32 imageIndex, uint32 passIndex, PipelineLayout* pPipelineLayout) 
	{
		m_DrawObjects.clear();
		m_pScene->OrderModels(m_DrawObjects);

		// Update descriptors
		FramePassKey framePassKey = {imageIndex, passIndex};
		for (uint32 drawObjectIndex = 0; auto& drawObjectPair : m_DrawObjects)
		{
			drawObjectPair.second.pDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex++, setIndex, pPipelineLayout);

			const Buffer* pVertexBuffer = drawObjectPair.second.UniqueMeshInstance.pMesh->GetVertexBuffer();
			drawObjectPair.second.pDescriptorSet->UpdateBufferBinding(0, pVertexBuffer, 0, pVertexBuffer->GetSize());
		}
	}

	void SceneRenderer::Render(const RenderContext& context)
	{
		CommandBuffer* commandBuffer = context.GetCommandBuffer();

		// One draw call for each unique mesh instance -> refill buffer for each draw call
		for (uint32 i = 0; auto& drawObject : m_DrawObjects)
		{
			uint32 instanceCount = drawObject.second.Matrices.size();

			// Fill uniform buffer
			// Buffer* pStagingBuffer = GetStagingBuffer(instanceCount, context.GetPassIndex());

			// Draw

			commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pDescriptorSet.get());

			Ref<Mesh> pMesh = drawObject.second.UniqueMeshInstance.pMesh;
			const Buffer* pVertexBuffer = pMesh->GetVertexBuffer();
			const Buffer* pIndexBuffer = pMesh->GetIndexBuffer();
			commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

			commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
		}
	}

	Ref<DescriptorSet> SceneRenderer::GetDescriptor(FramePassKey framePassKey, uint32 drawObjectIndex, uint32 setIndex, PipelineLayout* pPipelineLayout)
	{
		// The key and index already exists
		if (m_pVertexDescriptors.contains(framePassKey) && drawObjectIndex < m_pVertexDescriptors[framePassKey].size())
			return m_pVertexDescriptors[framePassKey][drawObjectIndex];

		// If either key or index is valid, make them valid and create descriptor
		m_pVertexDescriptors[framePassKey].push_back(RenderAPI::CreateDescriptorSet(pPipelineLayout, setIndex));
		return m_pVertexDescriptors[framePassKey].back();
	}
}