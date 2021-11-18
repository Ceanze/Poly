#include "SceneRenderer.h"

#include "Poly/Model/Mesh.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/Texture.h"
#include "Platform/API/Sampler.h"
#include "Platform/API/DescriptorSet.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Resources/ResourceManager.h"

namespace Poly
{
	void SceneRenderer::Update(const std::vector<SceneBinding>& sceneBindings, uint32 imageIndex, uint32 passIndex, PipelineLayout* pPipelineLayout)
	{
		m_DrawObjects.clear();
		m_pScene->OrderModels(m_DrawObjects);

		// Update descriptors
		auto vertexBinding = std::find_if(sceneBindings.begin(), sceneBindings.end(), [](const SceneBinding& binding){ return binding.Type == FResourceBindPoint::SCENE_VERTEX; });
		auto textureBinding = std::find_if(sceneBindings.begin(), sceneBindings.end(), [](const SceneBinding& binding){ return binding.Type == FResourceBindPoint::SCENE_TEXTURE; });
		auto instanceBinding = std::find_if(sceneBindings.begin(), sceneBindings.end(), [](const SceneBinding& binding){ return binding.Type == FResourceBindPoint::SCENE_INSTANCE; });
		for (uint32 drawObjectIndex = 0; auto& drawObjectPair : m_DrawObjects)
		{
			// SCENE_VERTEX
			if (vertexBinding != sceneBindings.end())
			{
				FramePassKey framePassKey = {imageIndex, passIndex, ResourceKey::VERTEX};
				drawObjectPair.second.pVertexDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, vertexBinding->SetIndex, pPipelineLayout);
				const Buffer* pVertexBuffer = drawObjectPair.second.UniqueMeshInstance.pMesh->GetVertexBuffer();
				drawObjectPair.second.pVertexDescriptorSet->UpdateBufferBinding(vertexBinding->Binding, pVertexBuffer, 0, pVertexBuffer->GetSize());
			}

			// SCENE_TEXTURE
			if (textureBinding != sceneBindings.end())
			{
				FramePassKey framePassKey = {imageIndex, passIndex, ResourceKey::TEXTURE};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, textureBinding->SetIndex, pPipelineLayout);
				TextureView* pTextureView = ResourceManager::GetTextureView(drawObjectPair.second.UniqueMeshInstance.MaterialID);
				drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(textureBinding->Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
			}

			// SCENE_INSTANCE
			if (instanceBinding != sceneBindings.end())
			{

			}

			drawObjectIndex++;
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
			if (drawObject.second.pVertexDescriptorSet)
				commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pVertexDescriptorSet.get());
			if (drawObject.second.pTextureDescriptorSet)
				commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pTextureDescriptorSet.get());

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