#include "RenderScene.h"

#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Scene/Scene.h"
#include "Poly/Scene/Components.h"
#include "Poly/Model/Material.h"
#include "Poly/Model/Mesh.h"

#include "Platform/API/Buffer.h" // TODO: See if this can be removed (needed to get Vertex buffer size)

namespace Poly
{

	RenderScene::RenderScene(Scene& scene, RenderGraphProgram& program)
		: m_Scene(scene), m_Program(program), m_TotalMeshCount(0) {}

	void RenderScene::Update()
	{
		m_SceneBatches.clear();
		m_InstanceHashToIndex.clear();
		m_TotalMeshCount = 0;

		// TODO: When/if possible, only go through those that are dirty and write that data instead of all of them
		auto view = m_Scene.m_Registry.view<MeshComponent, TransformComponent>();
		for (auto [entity, meshComp, transform] : view.each())
		{
			MeshInstance meshInstance = meshComp.pModel->GetMeshInstance(meshComp.MeshIndex);
			size_t hash = meshInstance.GetUniqueHash();

			if (m_InstanceHashToIndex.contains(hash))
			{
				// TODO: If transform matrix is needed; add it here
				SceneBatch& batch = m_SceneBatches[m_InstanceHashToIndex[hash]];
				batch.InstanceCount++;
				batch.Matrices.push_back(transform.GetTransform());
			}
			else
			{
				m_InstanceHashToIndex[hash] = m_SceneBatches.size();
				m_SceneBatches.push_back({ meshInstance, 1, { transform.GetTransform()}});
			}

			m_TotalMeshCount++;
		}

		const std::string& resGroupName = m_Scene.GetResourceGroup().GetGroupName();

		const ResourceGUID instanceGUID = { resGroupName + ":" + Scene::INSTANCE_RESOURCE_NAME };
		const ResourceGUID materialGUID = { resGroupName + ":" + Scene::MATERIAL_RESOURCE_NAME };
		CreateBufferIfNecessary(instanceGUID, m_TotalMeshCount * sizeof(glm::mat4));
		CreateBufferIfNecessary(materialGUID, m_TotalMeshCount * sizeof(MaterialValues));

		uint64 processedBatchInstances = 0;
		for (uint32 batchIndex = 0; const SceneBatch& batch : m_SceneBatches)
		{
			// Instance Buffer
			uint64 instanceSize = batch.InstanceCount * sizeof(glm::mat4);
			m_Program.UpdateGraphResource(instanceGUID, instanceSize, batch.Matrices.data(), processedBatchInstances * sizeof(glm::mat4), batchIndex);

			// Material Buffer
			uint64 materialSize = sizeof(MaterialValues);
			m_Program.UpdateGraphResource(materialGUID, materialSize, batch.MeshInstance.pMaterial->GetMaterialValues(), batchIndex * materialSize, batchIndex);

			// Vertex buffer
			ResourceGUID vertexGUID = { resGroupName + ":" + Scene::VERTICES_RESOURCE_NAME };
			const Buffer* pVertexBuffer = batch.MeshInstance.pMesh->GetVertexBuffer();
			m_Program.UpdateGraphResource(vertexGUID, ResourceView{ pVertexBuffer, pVertexBuffer->GetSize(), 0 }, batchIndex);

			// Albedo
			ResourceGUID albedoGUID = { resGroupName + ":" + Scene::ALBEDO_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(albedoGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::ALBEDO), nullptr }, batchIndex);

			// Metallic
			ResourceGUID metallicGUID = { resGroupName + ":" + Scene::METALLIC_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(metallicGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::METALIC), nullptr }, batchIndex);

			// Normal
			ResourceGUID normalGUID = { resGroupName + ":" + Scene::NORMAL_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(normalGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::NORMAL), nullptr }, batchIndex);

			// Roughness
			ResourceGUID roughnessGUID = { resGroupName + ":" + Scene::ROUGHNESS_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(roughnessGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::ROUGHNESS), nullptr }, batchIndex);

			// AO
			ResourceGUID aoGUID = { resGroupName + ":" + Scene::AO_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(aoGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::AMBIENT_OCCLUSION), nullptr }, batchIndex);

			// Combined
			ResourceGUID combinedGUID = { resGroupName + ":" + Scene::COMBINED_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(combinedGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::COMBINED), nullptr }, batchIndex);

			batchIndex++;
			processedBatchInstances += batch.InstanceCount;
		}
	}

	const std::vector<SceneBatch>& RenderScene::GetBatches() const
	{
		return m_SceneBatches;
	}

	void RenderScene::CreateBufferIfNecessary(const ResourceGUID& bufferGUID, uint64 size)
	{
		if (!m_Program.HasResource(bufferGUID)) {
			m_Program.CreateResource(bufferGUID, size, nullptr, FBufferUsage::STORAGE_BUFFER);
		}
	}

}