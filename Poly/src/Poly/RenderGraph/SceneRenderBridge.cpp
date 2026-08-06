#include "SceneRenderBridge.h"

#include "Platform/API/Buffer.h"
#include "Platform/API/Sampler.h"
#include "Poly/Core/RenderAPI.h"
#include "Poly/Model/Mesh.h"
#include "Poly/RenderGraph/RenderProgramInstance.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/Resources/GeometryPool.h"
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
	{}

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

		for (const PendingBatch& batch : pendingBatches)
		{
			Mesh*       pMesh       = batch.Instance.pMesh.get();
			BufferRange vertexRange = pMesh->GetMeshRange().Vertices;
			BufferRange indexRange  = pMesh->GetMeshRange().Indices;

			if (meshRanges.contains(pMesh))
				continue;

			MeshRange range;
			range.BaseVertex = vertexRange.ElementOffset;
			range.BaseIndex  = indexRange.ElementOffset;
			range.IndexCount = indexRange.ElementCount;

			meshRanges[pMesh] = range;
			meshesToCopy.push_back({pMesh, range});
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

		UploadInstanceAndMaterialBuffers(instanceData, materialData);
	}

	Buffer* SceneRenderBridge::GetIndexBuffer() const
	{
		return ResourceManager::Resolve(GeometryPool::GetIndexBufferHandle());
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
				packedIndices[i] = ResourceManager::RegisterExternalTextureAndSampler(
				    pMaterial->GetTextureView(kMaterialTextureOrder[i]), ETextureLayout::SHADER_READ_ONLY_OPTIMAL,
				    Sampler::GetDefaultLinearSampler().get());
			}
			it = m_MaterialTextureCache.emplace(pMaterial, packedIndices).first;
		}

		std::memcpy(&data.TextureAlbedoIndex, it->second.data(), sizeof(data.TextureAlbedoIndex) * it->second.size());
		return data;
	}

	void SceneRenderBridge::UploadInstanceAndMaterialBuffers(const std::vector<GPUInstanceData>& instances, const std::vector<GPUMaterialData>& materials)
	{
		const uint64 instanceSize = sizeof(GPUInstanceData) * instances.size();
		m_InstanceBufferHandle    = ResourceManager::CreateStorageBuffer(instanceSize, EMemoryUsage::CPU_VISIBLE, "SceneRenderBridge.Instances");
		ResourceManager::UploadBufferData(m_InstanceBufferHandle, instances.data(), instanceSize);

		const uint64 materialSize = sizeof(GPUMaterialData) * materials.size();
		m_MaterialBufferHandle    = ResourceManager::CreateStorageBuffer(materialSize, EMemoryUsage::CPU_VISIBLE, "SceneRenderBridge.Materials");
		ResourceManager::UploadBufferData(m_MaterialBufferHandle, materials.data(), materialSize);

		m_pProgramInstance->UpdateResource(Scene::VERTICES_RESOURCE_NAME_2, GeometryPool::GetVertexBufferHandle());
		m_pProgramInstance->UpdateResource(Scene::INSTANCE_RESOURCE_NAME_2, m_InstanceBufferHandle);
		m_pProgramInstance->UpdateResource(Scene::MATERIAL_RESOURCE_NAME_2, m_MaterialBufferHandle);
	}
} // namespace Poly
