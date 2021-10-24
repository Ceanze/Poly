#include "polypch.h"
#include "Scene.h"

#include "Poly/Model/Mesh.h"
#include "Poly/Core/RenderAPI.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/DescriptorSet.h"
#include "Platform/API/PipelineLayout.h"
#include "Poly/Resources/ResourceManager.h"
#include "Poly/Rendering/RenderGraph/RenderContext.h"

namespace Poly
{
	void Scene::AddModel(PolyID model)
	{
		m_Models.push_back(model);
	}

	void Scene::RemoveModel(PolyID model)
	{
		auto it = std::find(m_Models.begin(), m_Models.end(), model);
		if (it == m_Models.end())
			POLY_CORE_WARN("Cannot remove modelID {}, model could not be found", model);

		m_Models.erase(it);
	}

	void Scene::Update(const RenderContext& context)
	{
		if (!m_pInstanceBuffer)
		{
			if (!m_Models.empty())
				HandleInstanceBufferSize(GetMatrixCount(), context.GetPassIndex());
			else
				return;
		}

		Ref<DescriptorSet> pOldSet = GetDescriptorSet(context);

		Ref<DescriptorSet> pNewSet = RenderAPI::CreateDescriptorSetCopy(pOldSet);
		m_DescriptorSetsToBeDestroyed[{context.GetImageIndex(), context.GetPassIndex()}].push_back(pOldSet);
		m_PassDescriptorSets[context.GetPassIndex()] = pNewSet;

		// TODO: Don't have this as hardcoded (binding and sizeof)
		pNewSet->UpdateBufferBinding(0, m_pInstanceBuffer.get(), 0, sizeof(glm::mat4));

	}

	void Scene::Execute(const RenderContext& context)
	{
		// Order models with unique mesh instances
		std::unordered_map<size_t, DrawObject> drawObjects;
		OrderModels(drawObjects);

		CommandBuffer* commandBuffer = context.GetCommandBuffer();

		// One draw call for each unique mesh instance -> refill buffer for each draw call
		for (uint32 i = 0; auto& drawObject : drawObjects)
		{
			uint32 instanceCount = drawObject.second.Matrices.size();
			uint32 offset = sizeof(glm::mat4) * i++;

			// Fill uniform buffer
			Buffer* pStagingBuffer = GetStagingBuffer(instanceCount, context.GetPassIndex());

			// För nästa gång:
			// * Planera en övergång till SSBO istället för att debugga VBO:en
			// 		- En idé är att ha descriptors i RenderGraphProgram som vanligt för automatisering och ha instance buffern i scene
			//		- Kan göra som Herman hade - skicka in descriptorseten till scene, men låt RenderGraphProgram hantera dem :)
			//		- För att göra det schmut så kan ju en INSTANCE binding i reflection skapa både Instance och Vertex descriptors
			// PROBLEM
			//	* En SSBO per unik mesh kräver dock en descriptor per sådan mesh...

			// Draw
			if (!drawObject.second.pDescriptorSet)
			{
				drawObject.second.pDescriptorSet = GetDescriptorSet(context);
				commandBuffer->BindDescriptor(context.GetActivePipeline(), drawObject.second.pDescriptorSet.get(), 1, &offset);
			}
			else
			{
				POLY_CORE_ERROR("Scene::Execute was called with passIndex {}, but no instance set has been set by that passIndex!", context.GetPassIndex());
			}

			Ref<Mesh> pMesh = drawObject.second.UniqueMeshInstance.pMesh;
			const Buffer* pVertexBuffer = pMesh->GetVertexBuffer();
			const Buffer* pIndexBuffer = pMesh->GetIndexBuffer();
			commandBuffer->BindVertexBuffer(pVertexBuffer, 0, 1, 0);
			commandBuffer->BindIndexBuffer(pIndexBuffer, 0, EIndexType::UINT32);

			commandBuffer->DrawIndexedInstanced(pMesh->GetIndexCount(), instanceCount, 0, 0, 0);
		}
	}

	void Scene::OrderModels(std::unordered_map<size_t, DrawObject>& drawObjects)
	{
		for (auto modelID : m_Models)
		{
			Model* model = ResourceManager::GetModel(modelID);
			auto& meshInstances = model->GetMeshInstances();
			for (auto& meshInstance : meshInstances)
			{
				size_t hash = meshInstance.GetUniqueHash();

				if(!drawObjects.contains(hash))
				{
					drawObjects[hash].UniqueMeshInstance = meshInstance;
					drawObjects[hash].Matrices.push_back(model->GetTransform()); /** TODO: Multiply with the mesh matrix aswell when added **/
				}
				else
				{
					drawObjects[hash].Matrices.push_back(model->GetTransform());
				}
			}
		}
	}

	Buffer* Scene::GetStagingBuffer(uint32 matrixCount, uint32 passIndex)
	{
		HandleInstanceBufferSize(matrixCount, passIndex);

		uint32 size = matrixCount * sizeof(glm::mat4);

		if (m_StagingBuffers.contains(m_FrameIndex))
		{
			Ref<Buffer> pStagingBuffer = m_StagingBuffers[m_FrameIndex];
			if (pStagingBuffer->GetSize() >= size)
				return pStagingBuffer.get();
			else
			{
				m_BuffersToBeDestroyed[{m_FrameIndex, passIndex}].push_back(pStagingBuffer);
			}
		}

		BufferDesc desc = {};
		desc.BufferUsage	= FBufferUsage::TRANSFER_SRC;
		desc.MemUsage		= EMemoryUsage::CPU_VISIBLE;
		desc.Size			= size;
		Ref<Buffer> pStagingBuffer = RenderAPI::CreateBuffer(&desc);
		m_StagingBuffers[m_FrameIndex] = pStagingBuffer;

		return pStagingBuffer.get();
	}

	void Scene::HandleInstanceBufferSize(uint32 matrixCount, uint32 passIndex)
	{
		uint32 size = matrixCount * sizeof(glm::mat4);

		BufferDesc desc = {};
		desc.BufferUsage	= FBufferUsage::UNIFORM_BUFFER | FBufferUsage::TRANSFER_DST;
		desc.MemUsage		= EMemoryUsage::GPU_ONLY;
		desc.Size			= size;

		if (!m_pInstanceBuffer) // Create buffer
		{
			m_pInstanceBuffer = RenderAPI::CreateBuffer(&desc);
			return;
		}

		if (size > m_pInstanceBuffer->GetSize() || size < (m_pInstanceBuffer->GetSize() / 2))
		{
			m_BuffersToBeDestroyed[{m_FrameIndex, passIndex}].push_back(m_pInstanceBuffer);
			m_pInstanceBuffer = RenderAPI::CreateBuffer(&desc);
			return;
		}
	}

	uint32 Scene::GetMatrixCount(const std::unordered_map<size_t, DrawObject>& drawObjects)
	{
		uint32 count = 0;
		for (auto& drawObject : drawObjects)
			count += drawObject.second.Matrices.size();
		return count;
	}

	uint32 Scene::GetMatrixCount()
	{
		uint32 count = 0;
		for (auto modelID : m_Models)
		{
			Model* model = ResourceManager::GetModel(modelID);
			auto& meshInstances = model->GetMeshInstances();
			count += meshInstances.size();
		}

		return count;
	}

	Ref<DescriptorSet> Scene::GetDescriptorSet(const RenderContext& context)
	{
		uint32 passIndex = context.GetPassIndex();
		if (m_PassDescriptorSets.contains(passIndex))
			return m_PassDescriptorSets[passIndex];

		m_PassDescriptorSets[passIndex] = RenderAPI::CreateDescriptorSet(context.GetActivePipelineLayout(), context.GetInstanceSetIndex());
		return m_PassDescriptorSets[passIndex];
	}

}