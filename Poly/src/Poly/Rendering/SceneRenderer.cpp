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
	void SceneRenderer::Execute(const RenderContext& context, const Scene& scene, const PassReflection& reflection, uint32 imageIndex)
	{
		Update(context, scene, reflection, imageIndex);
		Render(context);
	}

	void SceneRenderer::Update(const RenderContext& context, const Scene& scene, const PassReflection& reflection, uint32 imageIndex)
	{
		uint32 passIndex = context.GetPassIndex();

		m_DrawObjects.clear();
		OrderModels(scene);

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
			// INSTANCE
			if (hasInstanceBuffer)
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::INSTANCE);
				uint64 size = drawObjectPair.second.Matrices.size() * sizeof(glm::mat4);
				drawObjectPair.second.pInstanceDescriptorSet = context.GetDescriptorCache()->GetDescriptorSet(ioData.Set, drawObjectIndex, imageIndex, 1, DescriptorCache::EAction::GET_OR_CREATE);
				drawObjectPair.second.pInstanceDescriptorSet->UpdateBufferBinding(ioData.Binding, m_pInstanceBuffer.get(), instanceOffset, size);

				m_pStagingBuffer->TransferData(drawObjectPair.second.Matrices.data(), size, instanceOffset);

				instanceOffset += size;
			}

			// MATERIAL
			if (hasMaterial)
			{
				const IOData& ioData = reflection.GetSceneBinding(ESceneBinding::MATERIAL);
				uint64 size = sizeof(MaterialValues);
				drawObjectPair.second.pMaterialDescriptorSet = context.GetDescriptorCache()->GetDescriptorSet(ioData.Set, drawObjectIndex, imageIndex, 1, DescriptorCache::EAction::GET_OR_CREATE);
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
		for (uint32 i = 0; auto& drawObject : m_DrawObjects) // TODO: This should get the scene draw data
		{
			uint32 instanceCount = static_cast<uint32>(drawObject.second.Matrices.size());

			// Draw
			// TODO: This will be removed when instanced drawing is introduced from the render graph program. Descriptor getting will be automated then
			std::set<DescriptorSet*> sets = { 
											  drawObject.second.pInstanceDescriptorSet, drawObject.second.pMaterialDescriptorSet};
			sets.insert(context.GetDescriptorCache()->GetDescriptorSet(3, i, DescriptorCache::EAction::GET));
			sets.insert(context.GetDescriptorCache()->GetDescriptorSet(4, i, DescriptorCache::EAction::GET));
			for (auto& set : sets)
				commandBuffer->BindDescriptor(context.GetActivePipeline(), set);


			Ref<Mesh> pMesh = drawObject.second.UniqueMeshInstance.pMesh;
			const Buffer* pIndexBuffer = pMesh->GetIndexBuffer();
			commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

			commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
		}
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

	void SceneRenderer::OrderModels(const Scene& scene)
	{
		// TODO: Think of the order things are drawn, in a scene hierarchy the root should be drawn first (probably?)
		// TODO: The transforms need to be applied in the correct order when having a deeper hierarchy in order for the transforms to be correct
		// TODO: The transform from assimp is per pNode, not per pMesh, fix resource loader storage of the
		//		 transforms and also handle that here. ---- This comment might be outdated. Currently, for each pMesh a new Entity is created. The separetion of pNode and pMesh should already be done
		// TODO: The transforms from assimp are relative to the parent pNode. This means that the ones that are saved for the model are local meshes. For rendering (and any other calculations) it is 
		//		 needed to multiply with the parent in ordder to achieve the correct transformation.
		auto view = scene.m_Registry.view<MeshComponent, TransformComponent>();
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

// 1. Scene on Update updates all "dirty" (tagged by a DirtyTag?) entities with their data (aka Transform, Textures, Materials) or if they are new to a Vector and map in scene
//		Transform and Material: Written directly to buffer or to temp buffer? Refine this! To start with for testing: Update the whole buffer everytime something is marked as dirty
//		Map: Contains a "MeshInstanceHash" to Index mapping. The MeshInstanceHash guarantees only one Instance data per unique instance
//		Vector: Contains the data per instance, condensed storage. Data per instance is at least {ResourceIndex (For RenderGraph), InstanceCount, pMesh}
//		Update: Calls UpdateRenderGraphResource for textures, and for data in buffers for Material and Instance, with index provided by the Map/Vector logic. This will be placed in Scene and the Update will take in the RenderGraphProgram as a parameter for that function.
// 2. Scene Renderer gets the vector from the scene during rendering call to loop through the render data
//		Scene renderer needs to somehow get the correct descriptors for the current index; to start with a simple getter in the RenderContext should suffice. <- Getter already exist to get the DescriptorCache.