#include "Scene.h"
#include "Entity.h"
#include "Components.h"
#include "Poly/Rendering/RenderGraph/RenderGraphProgram.h"
#include "Poly/Model/Model.h"
#include "Poly/Model/Mesh.h"
#include "Poly/Model/Material.h"

#include "Platform/API/Buffer.h" // TODO: See if this can be removed (needed to get Vertex buffer size)

namespace Poly
{
	Scene::Scene(const std::string& name) : m_ResourceGroup("scene"), m_Name(name)
	{
		m_ResourceGroup.AddResource(VERTICES_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(INSTANCE_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(MATERIAL_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(ALBEDO_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(NORMAL_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(COMBINED_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(METALLIC_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(ROUGHNESS_TEX_RESOURCE_NAME, false);
		m_ResourceGroup.AddResource(AO_TEX_RESOURCE_NAME, false);
	}

	Entity Scene::CreateEntity()
	{
		return CreateEntityWithID(PolyID());
	}

	Entity Scene::CreateEntityWithID(PolyID id)
	{
		entt::entity entity = m_Registry.create();

		m_Registry.emplace<TransformComponent>(entity);
		m_Registry.emplace<HierarchyComponent>(entity);
		m_Registry.emplace<IDComponent>(entity, id);
		m_Registry.emplace<DirtyTag>(entity);

		return Entity(this, entity);
	}

	Entity Scene::GetOrCreateEntityWithID(PolyID id)
	{
		auto view = m_Registry.view<const IDComponent>();

		entt::entity enttEntity = entt::null;
		for (auto [entity, IDComp] : view.each())
		{
			if (IDComp.ID == id)
			{
				enttEntity = entity;
				break;
			}
		}

		if (enttEntity == entt::null)
			return CreateEntityWithID(id);

		return Entity(this, enttEntity);
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	void Scene::Update(RenderGraphProgram& program)
	{
		// TODO: First check if any pass is using any binding by the scene

		// Only update when there are dirty entities
		if (m_Registry.storage<DirtyTag>().empty())
			return;
		m_DrawData.clear();
		m_InstanceHashToIndex.clear();

		// TODO: When/if possible, only go through those that are dirty and write that data instead of all of them
		auto view = m_Registry.view<MeshComponent, TransformComponent>();
		for (auto [entity, meshComp, transform] : view.each())
		{
			MeshInstance meshInstance = meshComp.pModel->GetMeshInstance(meshComp.MeshIndex);
			size_t hash = meshInstance.GetUniqueHash();

			if (m_InstanceHashToIndex.contains(hash))
			{
				// TODO: If transform matrix is needed; add it here
				m_DrawData[m_InstanceHashToIndex[hash]].InstanceCount++;
			}
			else
			{
				m_InstanceHashToIndex[hash] = m_DrawData.size();
				m_DrawData.push_back({ meshInstance, 0 });
			}
		}

		for (uint32 i = 0; const auto& drawData : m_DrawData)
		{
			// Vertex buffer
			ResourceGUID vertexGUID = { m_ResourceGroup.GetGroupName() + ":" + VERTICES_RESOURCE_NAME };
			const Buffer* pVertexBuffer = drawData.MeshInstance.pMesh->GetVertexBuffer();
			program.UpdateGraphResource(vertexGUID, ResourceView{ pVertexBuffer, pVertexBuffer->GetSize(), 0}, 0, i);

			// Albedo
			ResourceGUID albedoGUID = { m_ResourceGroup.GetGroupName() + ":" + ALBEDO_TEX_RESOURCE_NAME };
			program.UpdateGraphResource(albedoGUID, ResourceView{ drawData.MeshInstance.pMaterial->GetTextureView(Material::Type::ALBEDO), nullptr}, 0, i);

			// Metallic
			ResourceGUID metallicGUID = { m_ResourceGroup.GetGroupName() + ":" + METALLIC_TEX_RESOURCE_NAME };
			program.UpdateGraphResource(metallicGUID, ResourceView{ drawData.MeshInstance.pMaterial->GetTextureView(Material::Type::METALIC), nullptr }, 0 ,i);

			// Normal
			ResourceGUID normalGUID = { m_ResourceGroup.GetGroupName() + ":" + NORMAL_TEX_RESOURCE_NAME };
			program.UpdateGraphResource(normalGUID, ResourceView{ drawData.MeshInstance.pMaterial->GetTextureView(Material::Type::NORMAL), nullptr }, 0, i);

			// Roughness
			ResourceGUID roughnessGUID = { m_ResourceGroup.GetGroupName() + ":" + ROUGHNESS_TEX_RESOURCE_NAME };
			program.UpdateGraphResource(roughnessGUID, ResourceView{ drawData.MeshInstance.pMaterial->GetTextureView(Material::Type::ROUGHNESS), nullptr }, 0, i);

			// AO
			ResourceGUID aoGUID = { m_ResourceGroup.GetGroupName() + ":" + AO_TEX_RESOURCE_NAME };
			program.UpdateGraphResource(aoGUID, ResourceView{ drawData.MeshInstance.pMaterial->GetTextureView(Material::Type::AMBIENT_OCCLUSION), nullptr }, 0, i);

			// Combined
			ResourceGUID combinedGUID = { m_ResourceGroup.GetGroupName() + ":" + COMBINED_TEX_RESOURCE_NAME };
			program.UpdateGraphResource(combinedGUID, ResourceView{ drawData.MeshInstance.pMaterial->GetTextureView(Material::Type::COMBINED), nullptr }, 0, i);

			i++;
		}

		// TODO: Check if storage can be cleared directly instead
		for (auto [entity, mesh, transform] : view.each())
			m_Registry.storage<DirtyTag>().remove(entity);
	}

	PolyID Scene::GetIdOfEntity(entt::entity entity)
	{
		if (entity == entt::null)
			return PolyID::None();

		if (m_Registry.valid(entity))
		{
			if (m_Registry.any_of<IDComponent>(entity))
				return m_Registry.get<IDComponent>(entity).ID;

			POLY_CORE_WARN("Cannot get ID of entity {}, entity does not have IDComponent", static_cast<uint32>(entity));
		}

		POLY_CORE_WARN("Cannot get entity {}, identifer is not valid", static_cast<uint32>(entity));
		return PolyID::None();
	}
}