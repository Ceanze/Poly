#include "SceneRenderBridge.h"

#include "Platform/API/Buffer.h"
#include "Poly/RenderGraph/RenderProgramInstance.h"
#include "Poly/RenderGraph/ResourceManager.h"
#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/MaterialAsset.h"
#include "Poly/Resources/AssetTypes/MeshAsset.h"
#include "Poly/Resources/AssetTypes/TextureAsset.h"
#include "Poly/Resources/GeometryPool.h"
#include "Poly/Scene/Components.h"
#include "Poly/Scene/Components/MaterialComponent.h"
#include "Poly/Scene/Components/MeshAssetComponent.h"
#include "Poly/Scene/Scene.h"

#include <cstring>

namespace
{
	constexpr Poly::Material::Type kMaterialTextureOrder[6] = {
	    Poly::Material::Type::ALBEDO, Poly::Material::Type::METALIC, Poly::Material::Type::NORMAL,
	    Poly::Material::Type::ROUGHNESS, Poly::Material::Type::AMBIENT_OCCLUSION, Poly::Material::Type::COMBINED};

	// TODO: Point this at a real default texture (white/flat-normal) once ResourceManager has one.
	constexpr uint32 kInvalidTextureIndex = ~0u;
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
			AssetHandle<MeshAsset>     MeshHandle;
			AssetHandle<MaterialAsset> MaterialHandle;
			std::vector<glm::mat4>     Transforms;
		};

		std::vector<PendingBatch>          pendingBatches;
		std::unordered_map<uint64, size_t> keyToBatchIndex;

		// TODO: When/if possible, only walk dirty entities instead of the whole registry every rebuild.
		auto view = m_Scene.m_Registry.view<MeshAssetComponent, MaterialComponent, TransformComponent>();
		for (auto [entity, mesh, material, transform] : view.each())
		{
			// Handles are dense packed (index|generation) ints, so this is a bijective batch key -
			// no hash collisions possible, unlike hashing the two asset pointers together.
			const uint64 key = (uint64(mesh.MeshHandle.Get()) << 32) | uint64(material.MaterialHandle.Get());

			auto it = keyToBatchIndex.find(key);
			if (it == keyToBatchIndex.end())
			{
				keyToBatchIndex[key] = pendingBatches.size();
				pendingBatches.push_back({mesh.MeshHandle, material.MaterialHandle, {transform.GetTransform()}});
			}
			else
			{
				pendingBatches[it->second].Transforms.push_back(transform.GetTransform());
			}
		}

		m_DrawBatches.clear();
		if (pendingBatches.empty())
			return;

		// Resolve unique materials - one GPUMaterialData row each.
		std::unordered_map<AssetHandle<MaterialAsset>, uint32> materialIndices;
		std::vector<GPUMaterialData>                           materialData;

		for (const PendingBatch& batch : pendingBatches)
		{
			if (materialIndices.contains(batch.MaterialHandle))
				continue;

			materialIndices[batch.MaterialHandle] = static_cast<uint32>(materialData.size());
			materialData.push_back(BuildMaterialData(batch.MaterialHandle));
		}

		// Lay out instances contiguously per batch and record each batch's draw parameters.
		std::vector<GPUInstanceData> instanceData;
		m_DrawBatches.reserve(pendingBatches.size());

		for (const PendingBatch& batch : pendingBatches)
		{
			MeshAsset* pMeshAsset = AssetHandler::Resolve(batch.MeshHandle);
			if (!pMeshAsset)
			{
				POLY_CORE_WARN("Skipping batch - mesh asset handle {} is no longer valid", batch.MeshHandle.Get());
				continue;
			}

			const MeshRange& range       = pMeshAsset->GetMeshRange();
			const uint32     materialIdx = materialIndices[batch.MaterialHandle];

			SceneDrawBatch drawBatch;
			drawBatch.BaseVertex    = range.Vertices.ElementOffset;
			drawBatch.BaseIndex     = range.Indices.ElementOffset;
			drawBatch.IndexCount    = range.Indices.ElementCount;
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

	GPUMaterialData SceneRenderBridge::BuildMaterialData(AssetHandle<MaterialAsset> materialHandle)
	{
		GPUMaterialData data = {};

		MaterialAsset* pMaterialAsset = AssetHandler::Resolve(materialHandle);
		if (!pMaterialAsset)
		{
			POLY_CORE_WARN("Skipping material data build - material asset handle {} is no longer valid", materialHandle.Get());
			return data;
		}

		data.Values = pMaterialAsset->GetValues();

		auto it = m_MaterialTextureCache.find(materialHandle);
		if (it == m_MaterialTextureCache.end())
		{
			std::array<uint32, 6> packedIndices;
			for (uint32 i = 0; i < 6; i++)
				packedIndices[i] = PackTextureIndex(pMaterialAsset->GetTexture(kMaterialTextureOrder[i]));

			it = m_MaterialTextureCache.emplace(materialHandle, packedIndices).first;
		}

		std::memcpy(&data.TextureAlbedoIndex, it->second.data(), sizeof(data.TextureAlbedoIndex) * it->second.size());
		return data;
	}

	uint32 SceneRenderBridge::PackTextureIndex(AssetHandle<TextureAsset> textureHandle)
	{
		TextureAsset* pTextureAsset = AssetHandler::Resolve(textureHandle);
		if (!pTextureAsset)
			return kInvalidTextureIndex;

		const SamplerHandle samplerHandle = ResourceManager::GetDefaultLinearSampler();
		return pTextureAsset->GetHandle().GetIndex() | (samplerHandle.GetIndex() << ResourceManager::SAMPLER_INDEX_SHIFT);
	}

	void SceneRenderBridge::UploadInstanceAndMaterialBuffers(const std::vector<GPUInstanceData>& instances, const std::vector<GPUMaterialData>& materials)
	{
		const uint64 instanceSize = sizeof(GPUInstanceData) * instances.size();
		if (!m_InstanceBufferHandle.IsValid())
			m_InstanceBufferHandle = ResourceManager::CreateStorageBuffer(instanceSize, EMemoryUsage::CPU_VISIBLE, "SceneRenderBridge.Instances");
		else
			m_InstanceBufferHandle = ResourceManager::ResizeBuffer(m_InstanceBufferHandle, instanceSize);
		ResourceManager::UploadBufferData(m_InstanceBufferHandle, instances.data(), instanceSize);

		const uint64 materialSize = sizeof(GPUMaterialData) * materials.size();
		if (!m_MaterialBufferHandle.IsValid())
			m_MaterialBufferHandle = ResourceManager::CreateStorageBuffer(materialSize, EMemoryUsage::CPU_VISIBLE, "SceneRenderBridge.Materials");
		else
			m_MaterialBufferHandle = ResourceManager::ResizeBuffer(m_MaterialBufferHandle, materialSize);
		ResourceManager::UploadBufferData(m_MaterialBufferHandle, materials.data(), materialSize);

		m_pProgramInstance->UpdateResource(Scene::VERTICES_RESOURCE_NAME_2, GeometryPool::GetVertexBufferHandle());
		m_pProgramInstance->UpdateResource(Scene::INSTANCE_RESOURCE_NAME_2, m_InstanceBufferHandle);
		m_pProgramInstance->UpdateResource(Scene::MATERIAL_RESOURCE_NAME_2, m_MaterialBufferHandle);
	}
} // namespace Poly
