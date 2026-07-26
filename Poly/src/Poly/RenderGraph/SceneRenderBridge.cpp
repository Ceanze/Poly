#include "SceneRenderBridge.h"

#include "BindlessManager.h"
#include "Platform/API/Buffer.h"
#include "Platform/API/CommandBuffer.h"
#include "Platform/API/CommandPool.h"
#include "Platform/API/CommandQueue.h"
#include "Platform/API/Sampler.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Model/Mesh.h"
#include "Poly/RenderGraph/RenderProgramInstance.h"
#include "Poly/Scene/Components.h"
#include "Poly/Scene/Scene.h"

#include <cstring>

namespace
{
	constexpr Poly::Material::Type kMaterialTextureOrder[6] = {
	    Poly::Material::Type::ALBEDO, Poly::Material::Type::METALIC, Poly::Material::Type::NORMAL,
	    Poly::Material::Type::ROUGHNESS, Poly::Material::Type::AMBIENT_OCCLUSION, Poly::Material::Type::COMBINED};
} // namespace

namespace Poly
{
	SceneRenderBridge::SceneRenderBridge(Scene& scene, Ref<RenderProgramInstance> pProgramInstance)
	    : m_Scene(scene)
	    , m_pProgramInstance(std::move(pProgramInstance))
	{
		m_pCopyCommandPool   = RenderAPI::CreateCommandPool(FQueueType::GRAPHICS, FCommandPoolFlags::RESET_COMMAND_BUFFERS);
		m_pCopyCommandBuffer = m_pCopyCommandPool->AllocateCommandBuffer(ECommandBufferLevel::PRIMARY);
	}

	void SceneRenderBridge::Update()
	{
		struct PendingBatch
		{
			MeshInstance           Instance;
			std::vector<glm::mat4> Transforms;
		};

		std::vector<PendingBatch>          pendingBatches;
		std::unordered_map<size_t, size_t> hashToBatchIndex;

		// TODO: When/if possible, only walk dirty entities instead of the whole registry every rebuild.
		auto view = m_Scene.m_Registry.view<MeshComponent, TransformComponent>();
		for (auto [entity, meshComp, transform] : view.each())
		{
			MeshInstance instance = meshComp.pModel->GetMeshInstance(meshComp.MeshIndex);
			const size_t hash     = instance.GetUniqueHash();

			auto it = hashToBatchIndex.find(hash);
			if (it == hashToBatchIndex.end())
			{
				hashToBatchIndex[hash] = pendingBatches.size();
				pendingBatches.push_back({instance, {transform.GetTransform()}});
			}
			else
			{
				pendingBatches[it->second].Transforms.push_back(transform.GetTransform());
			}
		}

		m_DrawBatches.clear();
		if (pendingBatches.empty())
			return;

		// Resolve unique meshes - copy each mesh's GPU vertex/index data into the combined buffers exactly
		// once, in first-seen order, recording where each mesh's slice ends up.
		std::unordered_map<Mesh*, MeshRange>     meshRanges;
		std::vector<std::pair<Mesh*, MeshRange>> meshesToCopy;
		uint32                                   totalVertices = 0;
		uint32                                   totalIndices  = 0;

		for (const PendingBatch& batch : pendingBatches)
		{
			Mesh* pMesh = batch.Instance.pMesh.get();
			if (meshRanges.contains(pMesh))
				continue;

			MeshRange range;
			range.BaseVertex = totalVertices;
			range.BaseIndex  = totalIndices;
			range.IndexCount = pMesh->GetIndexCount();

			meshRanges[pMesh] = range;
			meshesToCopy.push_back({pMesh, range});

			totalVertices += pMesh->GetVertexCount();
			totalIndices += pMesh->GetIndexCount();
		}

		// Resolve unique materials - one GPUMaterialData row each.
		std::unordered_map<Material*, uint32> materialIndices;
		std::vector<GPUMaterialData>          materialData;

		for (const PendingBatch& batch : pendingBatches)
		{
			Material* pMaterial = batch.Instance.pMaterial.get();
			if (materialIndices.contains(pMaterial))
				continue;

			materialIndices[pMaterial] = static_cast<uint32>(materialData.size());
			materialData.push_back(BuildMaterialData(pMaterial));
		}

		// Lay out instances contiguously per batch and record each batch's draw parameters.
		std::vector<GPUInstanceData> instanceData;
		m_DrawBatches.reserve(pendingBatches.size());

		for (const PendingBatch& batch : pendingBatches)
		{
			const MeshRange& range       = meshRanges[batch.Instance.pMesh.get()];
			const uint32     materialIdx = materialIndices[batch.Instance.pMaterial.get()];

			SceneDrawBatch drawBatch;
			drawBatch.BaseVertex    = range.BaseVertex;
			drawBatch.BaseIndex     = range.BaseIndex;
			drawBatch.IndexCount    = range.IndexCount;
			drawBatch.FirstInstance = static_cast<uint32>(instanceData.size());
			drawBatch.InstanceCount = static_cast<uint32>(batch.Transforms.size());
			m_DrawBatches.push_back(drawBatch);

			for (const glm::mat4& transform : batch.Transforms)
				instanceData.push_back(GPUInstanceData{transform, materialIdx});
		}

		RebuildCombinedMeshBuffers(meshesToCopy, totalVertices, totalIndices);
		UploadInstanceAndMaterialBuffers(instanceData, materialData);
	}

	GPUMaterialData SceneRenderBridge::BuildMaterialData(Material* pMaterial)
	{
		GPUMaterialData data = {};
		data.Values          = *pMaterial->GetMaterialValues();

		auto it = m_MaterialTextureCache.find(pMaterial);
		if (it == m_MaterialTextureCache.end())
		{
			std::array<uint32, 6> packedIndices;
			for (uint32 i = 0; i < 6; i++)
			{
				packedIndices[i] = BindlessManager::RegisterTextureAndSampler(
				    pMaterial->GetTextureView(kMaterialTextureOrder[i]), ETextureLayout::SHADER_READ_ONLY_OPTIMAL,
				    Sampler::GetDefaultLinearSampler().get());
			}
			it = m_MaterialTextureCache.emplace(pMaterial, packedIndices).first;
		}

		std::memcpy(&data.TextureAlbedoIndex, it->second.data(), sizeof(data.TextureAlbedoIndex) * it->second.size());
		return data;
	}

	void SceneRenderBridge::RebuildCombinedMeshBuffers(const std::vector<std::pair<Mesh*, MeshRange>>& meshesToCopy, uint32 totalVertices, uint32 totalIndices)
	{
		BufferDesc vertexDesc  = {};
		vertexDesc.BufferUsage = FBufferUsage::TRANSFER_DST | FBufferUsage::STORAGE_BUFFER | FBufferUsage::SHADER_DEVICE_ADDRESS;
		vertexDesc.MemUsage    = EMemoryUsage::GPU_ONLY;
		vertexDesc.Size        = sizeof(Vertex) * totalVertices;
		m_pVertexBuffer        = RenderAPI::CreateBuffer(&vertexDesc);

		BufferDesc indexDesc  = {};
		indexDesc.BufferUsage = FBufferUsage::TRANSFER_DST | FBufferUsage::INDEX_BUFFER;
		indexDesc.MemUsage    = EMemoryUsage::GPU_ONLY;
		indexDesc.Size        = sizeof(uint32) * totalIndices;
		m_pIndexBuffer        = RenderAPI::CreateBuffer(&indexDesc);

		m_pCopyCommandPool->Reset();
		m_pCopyCommandBuffer->Begin(FCommandBufferFlag::ONE_TIME_SUBMIT);
		for (const auto& [pMesh, range] : meshesToCopy)
		{
			m_pCopyCommandBuffer->CopyBuffer(pMesh->GetVertexBuffer(), m_pVertexBuffer.get(), sizeof(Vertex) * pMesh->GetVertexCount(), 0,
			                                 sizeof(Vertex) * range.BaseVertex);
			m_pCopyCommandBuffer->CopyBuffer(pMesh->GetIndexBuffer(), m_pIndexBuffer.get(), sizeof(uint32) * pMesh->GetIndexCount(), 0,
			                                 sizeof(uint32) * range.BaseIndex);
		}
		m_pCopyCommandBuffer->End();

		// Blocking: this only runs when the scene's mesh/material set actually changed, not every frame
		SubmitDesc submitDesc     = {};
		submitDesc.CommandBuffers = {m_pCopyCommandBuffer};
		RenderAPI::GetCommandQueue(FQueueType::GRAPHICS)->SubmitIdle(submitDesc);
	}

	void SceneRenderBridge::UploadInstanceAndMaterialBuffers(const std::vector<GPUInstanceData>& instances, const std::vector<GPUMaterialData>& materials)
	{
		BufferDesc instanceDesc  = {};
		instanceDesc.BufferUsage = FBufferUsage::STORAGE_BUFFER | FBufferUsage::SHADER_DEVICE_ADDRESS;
		instanceDesc.MemUsage    = EMemoryUsage::CPU_VISIBLE;
		instanceDesc.Size        = sizeof(GPUInstanceData) * instances.size();
		m_pInstanceBuffer        = RenderAPI::CreateBuffer(&instanceDesc);
		m_pInstanceBuffer->TransferData(instances.data(), instanceDesc.Size, 0);

		BufferDesc materialDesc  = {};
		materialDesc.BufferUsage = FBufferUsage::STORAGE_BUFFER | FBufferUsage::SHADER_DEVICE_ADDRESS;
		materialDesc.MemUsage    = EMemoryUsage::CPU_VISIBLE;
		materialDesc.Size        = sizeof(GPUMaterialData) * materials.size();
		m_pMaterialBuffer        = RenderAPI::CreateBuffer(&materialDesc);
		m_pMaterialBuffer->TransferData(materials.data(), materialDesc.Size, 0);

		m_pProgramInstance->UpdateResource(Scene::VERTICES_RESOURCE_NAME_2, m_pVertexBuffer);
		m_pProgramInstance->UpdateResource(Scene::INSTANCE_RESOURCE_NAME_2, m_pInstanceBuffer);
		m_pProgramInstance->UpdateResource(Scene::MATERIAL_RESOURCE_NAME_2, m_pMaterialBuffer);
	}
} // namespace Poly
