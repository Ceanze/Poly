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
#include "Poly/Rendering/RenderGraph/PassReflection.h"

namespace Poly
{
	void SceneRenderer::Update(const RenderContext& context, const PassReflection& reflection, uint32 imageIndex, PipelineLayout* pPipelineLayout)
	{
		// TODO: Only update when the scene is "dirty" (or anything related is dirty)

		uint32 passIndex = context.GetPassIndex();

		m_DrawObjects.clear();
		OrderModels();

		bool hasInstanceBuffer	= reflection.HasSceneBinding(ESceneBinding::INSTANCE);
		bool hasMaterial	= reflection.HasSceneBinding(ESceneBinding::MATERIAL);
		if (hasInstanceBuffer || hasMaterial)
		{
			if (hasInstanceBuffer)
				UpdateInstanceBuffers(m_TotalMeshCount * sizeof(glm::mat4));
			if (hasMaterial)
				UpdateMaterialBuffers(m_TotalMeshCount * sizeof(MaterialValues));
		}

		uint64 instanceOffset = 0;
		uint64 materialOffset = 0;
		for (uint32 drawObjectIndex = 0; auto& drawObjectPair : m_DrawObjects)
		{
			// VERTEX
			if (reflection.HasSceneBinding(ESceneBinding::VERTEX))
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::VERTEX);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				drawObjectPair.second.pVertexDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				const Buffer* pVertexBuffer = drawObjectPair.second.UniqueMeshInstance.pMesh->GetVertexBuffer();
				drawObjectPair.second.pVertexDescriptorSet->UpdateBufferBinding(ioData.Binding, pVertexBuffer, 0, pVertexBuffer->GetSize());
			}

			// TEXTURE_ALBEDO
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_ALBEDO))
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::TEXTURE_ALBEDO);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				const TextureView* pTextureView = drawObjectPair.second.UniqueMeshInstance.pMaterial->GetTextureView(Material::Type::ALBEDO);
				drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(ioData.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
			}

			// TEXTURE_AO
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_AO))
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::TEXTURE_AO);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				const TextureView* pTextureView = drawObjectPair.second.UniqueMeshInstance.pMaterial->GetTextureView(Material::Type::AMBIENT_OCCLUSION);
				drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(ioData.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
			}

			// TEXTURE_COMBINED
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_COMBINED))
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::TEXTURE_COMBINED);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				const TextureView* pTextureView = drawObjectPair.second.UniqueMeshInstance.pMaterial->GetTextureView(Material::Type::COMBINED);
				drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(ioData.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
			}

			// TEXTURE_METALLIC
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_METALLIC))
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::TEXTURE_METALLIC);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				const TextureView* pTextureView = drawObjectPair.second.UniqueMeshInstance.pMaterial->GetTextureView(Material::Type::METALIC);
				drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(ioData.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
			}

			// TEXTURE_NORMAL
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_NORMAL))
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::TEXTURE_NORMAL);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				const TextureView* pTextureView = drawObjectPair.second.UniqueMeshInstance.pMaterial->GetTextureView(Material::Type::NORMAL);
				drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(ioData.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
			}

			// TEXTURE_ROUGHNESS
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_ROUGHNESS))
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::TEXTURE_ROUGHNESS);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				drawObjectPair.second.pTextureDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				const TextureView* pTextureView = drawObjectPair.second.UniqueMeshInstance.pMaterial->GetTextureView(Material::Type::ROUGHNESS);
				drawObjectPair.second.pTextureDescriptorSet->UpdateTextureBinding(ioData.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
			}

			// INSTANCE
			if (hasInstanceBuffer)
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::INSTANCE);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				uint64 size = drawObjectPair.second.Matrices.size() * sizeof(glm::mat4);
				drawObjectPair.second.pInstanceDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				drawObjectPair.second.pInstanceDescriptorSet->UpdateBufferBinding(ioData.Binding, m_pInstanceBuffer.get(), instanceOffset, size);

				m_pStagingBuffer->TransferData(drawObjectPair.second.Matrices.data(), size, instanceOffset);

				instanceOffset += size;
			}

			// MATERIAL
			if (hasMaterial)
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::MATERIAL);
				FramePassKey framePassKey = {imageIndex, passIndex, ioData.Set};
				uint64 size = sizeof(MaterialValues);
				drawObjectPair.second.pMaterialDescriptorSet = GetDescriptor(framePassKey, drawObjectIndex, ioData.Set, pPipelineLayout);
				drawObjectPair.second.pMaterialDescriptorSet->UpdateBufferBinding(ioData.Binding, m_pMaterialBuffer.get(), materialOffset, size);

				m_pMaterialStagingBuffer->TransferData(drawObjectPair.second.UniqueMeshInstance.pMaterial->GetMaterialValues(), size, materialOffset);

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

	void SceneRenderer::OrderModels()
	{
		// TODO: Think of the order things are drawn, in a scene hierarchy the root should be drawn first (probably?)
		// TODO: The transforms need to be applied in the correct order when having a deeper hierarchy in order for the transforms to be correct
		// TODO: The transform from assimp is per pNode, not per pMesh, fix resource loader storage of the
		//		 transforms and also handle that here
		auto view = m_pScene->m_Registry.view<MeshComponent, TransformComponent>();
		m_TotalMeshCount = 0;
		for (auto [entity, meshComp, transform] : view.each())
		{
			auto meshInstance = meshComp.pModel->GetMeshInstance(meshComp.MeshIndex);

			size_t hash = meshInstance.GetUniqueHash();

			m_DrawObjects[hash].UniqueMeshInstance = meshInstance;
			m_DrawObjects[hash].Matrices.push_back(transform.GetTransform());
			m_TotalMeshCount++;
		}
	}
}