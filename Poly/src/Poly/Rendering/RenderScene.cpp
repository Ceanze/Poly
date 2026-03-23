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
				m_InstanceHashToIndex[hash] = static_cast<uint32>(m_SceneBatches.size());
				m_SceneBatches.push_back({ meshInstance, 1, { transform.GetTransform()}});
			}

			m_TotalMeshCount++;
		}

		const std::string& resGroupName = m_Scene.GetResourceGroup().GetGroupName();

		const PassResID instanceID = ResID(resGroupName + ":" + Scene::INSTANCE_RESOURCE_NAME).GetAsExternal();
		const PassResID materialID = ResID(resGroupName + ":" + Scene::MATERIAL_RESOURCE_NAME).GetAsExternal();
		CreateBufferIfNecessary(instanceID, m_TotalMeshCount * sizeof(glm::mat4));
		CreateBufferIfNecessary(materialID, m_TotalMeshCount * sizeof(MaterialValues));

		uint64 processedBatchInstances = 0;
		for (uint32 batchIndex = 0; const SceneBatch& batch : m_SceneBatches)
		{
			// Instance Buffer
			uint64 instanceSize = batch.InstanceCount * sizeof(glm::mat4);
			m_Program.UpdateGraphResource(instanceID, instanceSize, batch.Matrices.data(), processedBatchInstances * sizeof(glm::mat4), batchIndex);

			// Material Buffer
			uint64 materialSize = sizeof(MaterialValues);
			m_Program.UpdateGraphResource(materialID, materialSize, batch.MeshInstance.pMaterial->GetMaterialValues(), batchIndex * materialSize, batchIndex);

			// Vertex buffer
			PassResID vertexID = ResID(resGroupName + ":" + Scene::VERTICES_RESOURCE_NAME).GetAsExternal();
			const Buffer* pVertexBuffer = batch.MeshInstance.pMesh->GetVertexBuffer();
			m_Program.UpdateGraphResource(vertexID, ResourceView{ pVertexBuffer, pVertexBuffer->GetSize(), 0 }, batchIndex);

			// Albedo
			PassResID albedoID = ResID(resGroupName + ":" + Scene::ALBEDO_TEX_RESOURCE_NAME).GetAsExternal();
			m_Program.UpdateGraphResource(albedoID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::ALBEDO), nullptr }, batchIndex);

			// Metallic
			PassResID metallicID = ResID(resGroupName + ":" + Scene::METALLIC_TEX_RESOURCE_NAME).GetAsExternal();
			m_Program.UpdateGraphResource(metallicID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::METALIC), nullptr }, batchIndex);

			// Normal
			PassResID normalID = ResID(resGroupName + ":" + Scene::NORMAL_TEX_RESOURCE_NAME).GetAsExternal();
			m_Program.UpdateGraphResource(normalID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::NORMAL), nullptr }, batchIndex);

			// Roughness
			PassResID roughnessID = ResID(resGroupName + ":" + Scene::ROUGHNESS_TEX_RESOURCE_NAME).GetAsExternal();
			m_Program.UpdateGraphResource(roughnessID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::ROUGHNESS), nullptr }, batchIndex);

			// AO
			PassResID aoID = ResID(resGroupName + ":" + Scene::AO_TEX_RESOURCE_NAME).GetAsExternal();
			m_Program.UpdateGraphResource(aoID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::AMBIENT_OCCLUSION), nullptr }, batchIndex);

			// Combined
			PassResID combinedID = ResID(resGroupName + ":" + Scene::COMBINED_TEX_RESOURCE_NAME).GetAsExternal();
			m_Program.UpdateGraphResource(combinedID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::COMBINED), nullptr }, batchIndex);

			batchIndex++;
			processedBatchInstances += batch.InstanceCount;
		}
	}

	const std::vector<SceneBatch>& RenderScene::GetBatches() const
	{
		return m_SceneBatches;
	}

	void RenderScene::CreateBufferIfNecessary(const PassResID& bufferID, uint64 size)
	{
		if (!m_Program.HasResource(bufferID)) {
			m_Program.CreateResource(bufferID.GetResource(), size, nullptr, FBufferUsage::STORAGE_BUFFER);
		}
	}

}