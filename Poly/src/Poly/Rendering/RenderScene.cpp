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
		: m_Scene(scene), m_Program(program) {}

	void RenderScene::Update()
	{
		m_SceneBatches.clear();
		m_InstanceHashToIndex.clear();

		// TODO: When/if possible, only go through those that are dirty and write that data instead of all of them
		auto view = m_Scene.m_Registry.view<MeshComponent, TransformComponent>();
		for (auto [entity, meshComp, transform] : view.each())
		{
			MeshInstance meshInstance = meshComp.pModel->GetMeshInstance(meshComp.MeshIndex);
			size_t hash = meshInstance.GetUniqueHash();

			if (m_InstanceHashToIndex.contains(hash))
			{
				// TODO: If transform matrix is needed; add it here
				m_SceneBatches[m_InstanceHashToIndex[hash]].InstanceCount++;
			}
			else
			{
				m_InstanceHashToIndex[hash] = m_SceneBatches.size();
				m_SceneBatches.push_back({ meshInstance, 0 });
			}
		}

		const std::string& resGroupName = m_Scene.GetResourceGroup().GetGroupName();

		for (uint32 i = 0; const auto& batch : m_SceneBatches)
		{
			// Vertex buffer
			ResourceGUID vertexGUID = { resGroupName + ":" + Scene::VERTICES_RESOURCE_NAME };
			const Buffer* pVertexBuffer = batch.MeshInstance.pMesh->GetVertexBuffer();
			m_Program.UpdateGraphResource(vertexGUID, ResourceView{ pVertexBuffer, pVertexBuffer->GetSize(), 0 }, 0, i);

			// Albedo
			ResourceGUID albedoGUID = { resGroupName + ":" + Scene::ALBEDO_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(albedoGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::ALBEDO), nullptr }, 0, i);

			// Metallic
			ResourceGUID metallicGUID = { resGroupName + ":" + Scene::METALLIC_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(metallicGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::METALIC), nullptr }, 0, i);

			// Normal
			ResourceGUID normalGUID = { resGroupName + ":" + Scene::NORMAL_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(normalGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::NORMAL), nullptr }, 0, i);

			// Roughness
			ResourceGUID roughnessGUID = { resGroupName + ":" + Scene::ROUGHNESS_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(roughnessGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::ROUGHNESS), nullptr }, 0, i);

			// AO
			ResourceGUID aoGUID = { resGroupName + ":" + Scene::AO_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(aoGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::AMBIENT_OCCLUSION), nullptr }, 0, i);

			// Combined
			ResourceGUID combinedGUID = { resGroupName + ":" + Scene::COMBINED_TEX_RESOURCE_NAME };
			m_Program.UpdateGraphResource(combinedGUID, ResourceView{ batch.MeshInstance.pMaterial->GetTextureView(Material::Type::COMBINED), nullptr }, 0, i);

			i++;
		}
	}

	const std::vector<SceneBatch>& RenderScene::GetBatches() const
	{
		return m_SceneBatches;
	}

}