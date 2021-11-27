#include "SceneRenderer.h"

#include "Poly/Model/Mesh.h"
#include "Poly/Model/Material.h"
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
	void SceneRenderer::Update(const RenderContext& context, const std::vector<SceneBinding>& sceneBindings, uint32 imageIndex, PipelineLayout* pPipelineLayout)
	{
		uint32 passIndex = context.GetPassIndex();

		m_DrawObjects.clear();
		m_pScene->OrderModels(m_DrawObjects);

		auto vertexBinding = std::find_if(sceneBindings.begin(), sceneBindings.end(), [](const SceneBinding& binding){ return binding.Type == FResourceBindPoint::SCENE_VERTEX; });
		auto textureBinding = std::find_if(sceneBindings.begin(), sceneBindings.end(), [](const SceneBinding& binding){ return binding.Type == FResourceBindPoint::SCENE_TEXTURES; });
		auto instanceBinding = std::find_if(sceneBindings.begin(), sceneBindings.end(), [](const SceneBinding& binding){ return binding.Type == FResourceBindPoint::SCENE_INSTANCE; });
		auto materialBinding = std::find_if(sceneBindings.begin(), sceneBindings.end(), [](const SceneBinding& binding){ return binding.Type == FResourceBindPoint::SCENE_MATERIAL; });

		bool hasInstanceBuffer	= instanceBinding != sceneBindings.end();
		bool hasMaterial		= materialBinding != sceneBindings.end();
		if (hasInstanceBuffer || hasMaterial)
		{
			uint32 totalInstanceCount = m_pScene->GetTotalMatrixCount(m_DrawObjects);
			if (hasInstanceBuffer)
				UpdateInstanceBuffers(totalInstanceCount * sizeof(glm::mat4));
			if (hasMaterial)
				UpdateMaterialBuffers(totalInstanceCount * sizeof(MaterialValues));
		}

		uint64 instanceOffset = 0;
		uint64 materialOffset = 0;
		for (uint32 drawObjectIndex = 0; auto& drawObjectPair : m_DrawObjects)
		{
			// SCENE_VERTEX
			if (vertexBinding != sceneBindings.end())
			{
				FramePassKey framePassKey = {imageIndex, passIndex, vertexBinding->SetIndex};
				drawObjectPair.second.pVertexDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, vertexBinding->SetIndex, pPipelineLayout);
				const Buffer* pVertexBuffer = drawObjectPair.second.UniqueMeshInstance.pMesh->GetVertexBuffer();
				drawObjectPair.second.pVertexDescriptorSet->UpdateBufferBinding(vertexBinding->Binding, pVertexBuffer, 0, pVertexBuffer->GetSize());
			}

			// SCENE_TEXTURE
			if (textureBinding != sceneBindings.end())
			{
				FramePassKey framePassKey = {imageIndex, passIndex, textureBinding->SetIndex};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, textureBinding->SetIndex, pPipelineLayout);

				for (uint32 i = 1; auto& binding : sceneBindings)
				{
					if (binding.Type == FResourceBindPoint::SCENE_TEXTURES)
					{
						// TODO: Make this more generic instead of relying on a specific order decided by enum
						const TextureView* pTextureView = ResourceManager::GetMaterial(drawObjectPair.second.UniqueMeshInstance.MaterialID)->GetTextureView(Material::Type(i++));
						drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(binding.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
					}
				}
			}

			// SCENE_INSTANCE
			if (hasInstanceBuffer)
			{
				FramePassKey framePassKey = {imageIndex, passIndex, instanceBinding->SetIndex};
				uint64 size = drawObjectPair.second.Matrices.size() * sizeof(glm::mat4);
				drawObjectPair.second.pInstanceDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, instanceBinding->SetIndex, pPipelineLayout);
				drawObjectPair.second.pInstanceDescriptorSet->UpdateBufferBinding(instanceBinding->Binding, m_pInstanceBuffer.get(), instanceOffset, size);

				m_pStagingBuffer->TransferData(drawObjectPair.second.Matrices.data(), size, instanceOffset);

				instanceOffset += size;
			}

			// SCENE_MATERIAL
			if (hasMaterial)
			{
				FramePassKey framePassKey = {imageIndex, passIndex, materialBinding->SetIndex};
				uint64 size = sizeof(MaterialValues);
				drawObjectPair.second.pMaterialDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, materialBinding->SetIndex, pPipelineLayout);
				drawObjectPair.second.pMaterialDescriptorSet->UpdateBufferBinding(materialBinding->Binding, m_pMaterialBuffer.get(), materialOffset, size);

				m_pMaterialStagingBuffer->TransferData(drawObjectPair.second.pMaterial->GetMaterialValues(), size, materialOffset);

				materialOffset += size;
			}

			drawObjectIndex++;
		}

		if (hasInstanceBuffer)
			context.GetCommandBuffer()->CopyBuffer(m_pStagingBuffer.get(), m_pInstanceBuffer.get(), m_pStagingBuffer->GetSize(), 0, 0);
		if (hasMaterial)
			context.GetCommandBuffer()->CopyBuffer(m_pMaterialStagingBuffer.get(), m_pMaterialBuffer.get(), m_pMaterialStagingBuffer->GetSize(), 0, 0);
	}

	void SceneRenderer::Render(const RenderContext& context)
	{
		CommandBuffer* commandBuffer = context.GetCommandBuffer();

		// One draw call for each unique mesh instance -> refill buffer for each draw call
		for (uint32 i = 0; auto& drawObject : m_DrawObjects)
		{
			uint32 instanceCount = drawObject.second.Matrices.size();

			// Draw
			std::set<DescriptorSet*> sets = { drawObject.second.pVertexDescriptorSet.get(), drawObject.second.pTextureDescriptorSet.get(),
											  drawObject.second.pInstanceDescriptorSet.get(), drawObject.second.pMaterialDescriptorSet.get()};
			// if (drawObject.second.pVertexDescriptorSet)
			// 	commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pVertexDescriptorSet.get());
			// if (drawObject.second.pTextureDescriptorSet)
			// 	commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pTextureDescriptorSet.get());
			// if (drawObject.second.pInstanceDescriptorSet)
			// 	commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pInstanceDescriptorSet.get());
			// if (drawObject.second.pMaterialDescriptorSet)
			// 	commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pMaterialDescriptorSet.get());
			for (auto& set : sets)
				commandBuffer->BindDescriptor(context.GetActivePipeline(), set);


			Ref<Mesh> pMesh = drawObject.second.UniqueMeshInstance.pMesh;
			const Buffer* pIndexBuffer = pMesh->GetIndexBuffer();
			commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

			commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
		}
	}

	Ref<DescriptorSet> SceneRenderer::GetDescriptor(FramePassKey framePassKey, uint32 drawObjectIndex, uint32 setIndex, PipelineLayout* pPipelineLayout)
	{
		// The key and index already exists
		if (m_Descriptors.contains(framePassKey) && drawObjectIndex < m_Descriptors[framePassKey].size())
			return m_Descriptors[framePassKey][drawObjectIndex];

		// If either key or index is valid, make them valid and create descriptor
		m_Descriptors[framePassKey].push_back(RenderAPI::CreateDescriptorSet(pPipelineLayout, setIndex));
		return m_Descriptors[framePassKey].back();
	}

	void SceneRenderer::UpdateInstanceBuffers(uint64 size)
	{
		if (size == 0)
		{
			POLY_CORE_WARN("UpdateBuffers was called with a size of 0, this should probably not be done, ignoring");
			return;
		}

		if (!m_pStagingBuffer || m_pStagingBuffer->GetSize() < size)
		{
			m_pStagingBuffer.reset();
			m_pInstanceBuffer.reset();

			BufferDesc desc = {};
			desc.BufferUsage	= FBufferUsage::STORAGE_BUFFER | FBufferUsage::TRANSFER_SRC;
			desc.MemUsage		= EMemoryUsage::CPU_VISIBLE;
			desc.Size			= size;
			m_pStagingBuffer = RenderAPI::CreateBuffer(&desc);

			desc.BufferUsage	= FBufferUsage::STORAGE_BUFFER | FBufferUsage::TRANSFER_DST;
			desc.MemUsage		= EMemoryUsage::GPU_ONLY;
			m_pInstanceBuffer = RenderAPI::CreateBuffer(&desc);
		}
	}

	void SceneRenderer::UpdateMaterialBuffers(uint64 size)
	{
		if (size == 0)
		{
			POLY_CORE_WARN("UpdateBuffers was called with a size of 0, this should probably not be done, ignoring");
			return;
		}

		if (!m_pMaterialStagingBuffer || m_pMaterialStagingBuffer->GetSize() < size)
		{
			m_pMaterialStagingBuffer.reset();
			m_pMaterialBuffer.reset();

			BufferDesc desc = {};
			desc.BufferUsage	= FBufferUsage::STORAGE_BUFFER | FBufferUsage::TRANSFER_SRC;
			desc.MemUsage		= EMemoryUsage::CPU_VISIBLE;
			desc.Size			= size;
			m_pMaterialStagingBuffer = RenderAPI::CreateBuffer(&desc);

			desc.BufferUsage	= FBufferUsage::STORAGE_BUFFER | FBufferUsage::TRANSFER_DST;
			desc.MemUsage		= EMemoryUsage::GPU_ONLY;
			m_pMaterialBuffer = RenderAPI::CreateBuffer(&desc);
		}
	}
}