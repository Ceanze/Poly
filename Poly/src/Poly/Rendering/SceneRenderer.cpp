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
// #include "Poly/Rendering/RenderGraph/PassReflection.h"

namespace Poly
{
	void SceneRenderer::Update(const RenderContext& context, const PassReflection& reflection, uint32 imageIndex, PipelineLayout* pPipelineLayout)
	{
		// TODO: Only update when the scene is "dirty" (or anything related is dirty)

		// TODO: Use GetDescriptorCopy and only have one descriptor per buffer (instead one per buffer per frame) - can only be sensibly done when this is only updated
		// when it is needed (read above TODO)

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
				drawObjectPair.second.pVertexDescriptorSet = context.GetDescriptorCache()->GetDescriptorSet(ioData.Set, drawObjectIndex, imageIndex, 1);
				const Buffer* pVertexBuffer = drawObjectPair.second.UniqueMeshInstance.pMesh->GetVertexBuffer();
				drawObjectPair.second.pVertexDescriptorSet->UpdateBufferBinding(ioData.Binding, pVertexBuffer, 0, pVertexBuffer->GetSize());
			}

			// TEXTURE_ALBEDO
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_ALBEDO))
				UpdateTextureDescriptor(context, reflection, drawObjectPair.second, drawObjectIndex, imageIndex, ESceneBinding::TEXTURE_ALBEDO, Material::Type::ALBEDO);


			// TEXTURE_AO
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_AO))
				UpdateTextureDescriptor(context, reflection, drawObjectPair.second, drawObjectIndex, imageIndex, ESceneBinding::TEXTURE_AO, Material::Type::AMBIENT_OCCLUSION);


			// TEXTURE_COMBINED
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_COMBINED))
				UpdateTextureDescriptor(context, reflection, drawObjectPair.second, drawObjectIndex, imageIndex, ESceneBinding::TEXTURE_COMBINED, Material::Type::COMBINED);


			// TEXTURE_METALLIC
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_METALLIC))
				UpdateTextureDescriptor(context, reflection, drawObjectPair.second, drawObjectIndex, imageIndex, ESceneBinding::TEXTURE_METALLIC, Material::Type::METALIC);


			// TEXTURE_NORMAL
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_NORMAL))
				UpdateTextureDescriptor(context, reflection, drawObjectPair.second, drawObjectIndex, imageIndex, ESceneBinding::TEXTURE_NORMAL, Material::Type::NORMAL);


			// TEXTURE_ROUGHNESS
			if (reflection.HasSceneBinding(ESceneBinding::TEXTURE_ROUGHNESS))
				UpdateTextureDescriptor(context, reflection, drawObjectPair.second, drawObjectIndex, imageIndex, ESceneBinding::TEXTURE_ROUGHNESS, Material::Type::ROUGHNESS);


			// INSTANCE
			if (hasInstanceBuffer)
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::INSTANCE);
				uint64 size = drawObjectPair.second.Matrices.size() * sizeof(glm::mat4);
				drawObjectPair.second.pInstanceDescriptorSet = context.GetDescriptorCache()->GetDescriptorSet(ioData.Set, drawObjectIndex, imageIndex, 1);
				drawObjectPair.second.pInstanceDescriptorSet->UpdateBufferBinding(ioData.Binding, m_pInstanceBuffer.get(), instanceOffset, size);

				m_pStagingBuffer->TransferData(drawObjectPair.second.Matrices.data(), size, instanceOffset);

				instanceOffset += size;
			}

			// MATERIAL
			if (hasMaterial)
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::MATERIAL);
				uint64 size = sizeof(MaterialValues);
				drawObjectPair.second.pMaterialDescriptorSet = context.GetDescriptorCache()->GetDescriptorSet(ioData.Set, drawObjectIndex, imageIndex, 1);
				drawObjectPair.second.pMaterialDescriptorSet->UpdateBufferBinding(ioData.Binding, m_pMaterialBuffer.get(), materialOffset, size);

				m_pMaterialStagingBuffer->TransferData(drawObjectPair.second.UniqueMeshInstance.pMaterial->GetMaterialValues(), size, materialOffset);

				materialOffset += size;
			}

			drawObjectIndex++;
		}

		if (hasInstanceBuffer && m_pStagingBuffer)
			context.GetCommandBuffer()->CopyBuffer(m_pStagingBuffer.get(), m_pInstanceBuffer.get(), m_pStagingBuffer->GetSize(), 0, 0);
		if (hasMaterial && m_pMaterialStagingBuffer)
			context.GetCommandBuffer()->CopyBuffer(m_pMaterialStagingBuffer.get(), m_pMaterialBuffer.get(), m_pMaterialStagingBuffer->GetSize(), 0, 0);
	}

	void SceneRenderer::Render(const RenderContext& context)
	{
		CommandBuffer* commandBuffer = context.GetCommandBuffer();

		// One draw call for each unique mesh instance -> refill buffer for each draw call
		for (uint32 i = 0; auto& drawObject : m_DrawObjects)
		{
			uint32 instanceCount = static_cast<uint32>(drawObject.second.Matrices.size());

			// Draw
			std::set<DescriptorSet*> sets = { drawObject.second.pVertexDescriptorSet, drawObject.second.pTextureDescriptorSet,
											  drawObject.second.pInstanceDescriptorSet, drawObject.second.pMaterialDescriptorSet};
			for (auto& set : sets)
				commandBuffer->BindDescriptor(context.GetActivePipeline(), set);


			Ref<Mesh> pMesh = drawObject.second.UniqueMeshInstance.pMesh;
			const Buffer* pIndexBuffer = pMesh->GetIndexBuffer();
			commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

			commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
		}
	}

	void SceneRenderer::UpdateTextureDescriptor(const RenderContext& context, const PassReflection& reflection, DrawObject& drawObject, uint32 drawObjectIndex, uint32 imageIndex, ESceneBinding sceneBinding, Material::Type type)
	{
		const IOData& ioData = reflection.GetSceneBinding(sceneBinding);
		drawObject.pTextureDescriptorSet = context.GetDescriptorCache()->GetDescriptorSet(ioData.Set, drawObjectIndex, imageIndex, 1);
		const TextureView* pTextureView = drawObject.UniqueMeshInstance.pMaterial->GetTextureView(type);
		drawObject.pTextureDescriptorSet->UpdateTextureBinding(ioData.Binding, ETextureLayout::SHADER_READ_ONLY_OPTIMAL, pTextureView, Sampler::GetDefaultLinearSampler().get());
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
		//		 transforms and also handle that here. ---- This comment might be outdated. Currently, for each pMesh a new Entity is created. The separetion of pNode and pMesh should already be done
		// TODO: The transforms from assimp are relative to the parent pNode. This means that the ones that are saved for the model are local meshes. For rendering (and any other calculations) it is 
		//		 needed to multiply with the parent in ordder to achieve the correct transformation.
		if (!m_pScene)
			return;

		auto view = m_pScene->m_Registry.view<MeshComponent, TransformComponent>();
		m_TotalMeshCount = 0;
		for (auto [entity, meshComp, transform] : view.each())
		{
			auto meshInstance = meshComp.pModel->GetMeshInstance(meshComp.MeshIndex);

			size_t hash = meshInstance.GetUniqueHash();

			m_DrawObjects[hash].UniqueMeshInstance = meshInstance;
			m_DrawObjects[hash].Matrices.push_back(transform.GetTransform()); // TODO: Here I am copying the data already! Either send this to a staging buffer right here and now, or use this data when sending to UpdateGraphResource(data*); mDrawObjects should probably be a std::vector in the case of UpdateGraphResource
			m_TotalMeshCount++;
		}
	}
}

/*
* SceneRenderer and Scene structure breakdown:
* 
* Currently each entity in the ENTT view of the scene is being rendered without any given order. The scene hierarchy is not followed.
* 
* What should instead be done is that, from the root entity in the scene, each entity from that is rendered in order following an algorithm such as DFS.
* To do this two main things need to change:
*	1. A root node needs to be defined and saved for the renderer to access. Should ideally be saved in the Scene as a special member.
*	2. From the root node the renderer should follow the children in order of the hierachy. This order is used to define the transformation data. The rendering itself will still be based "randomly" based on the mesh instance.
*/