#include "Scene.h"

#include "Components.h"
#include "Components/MaterialComponent.h"
#include "Components/MeshAssetComponent.h"
#include "Poly/RenderGraph/SceneRenderBridge.h"
#include "Poly/Rendering/RenderScene.h"
#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/Scene/Entity.h"

namespace Poly
{
	Scene::Scene(const std::string& name)
	    : m_ResourceGroup("scene")
	    , m_Name(name)
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

		m_Registry.ctx().emplace<Scene*>(this);
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

		return Entity({m_Registry, entity});
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

		return Entity({m_Registry, enttEntity});
	}

	void Scene::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	Entity Scene::InstantiateSceneAsset(AssetHandle<SceneAsset> sceneAssetHandle, Entity parent)
	{
		SceneAsset* pSceneAsset = AssetHandler::Resolve(sceneAssetHandle);
		if (!pSceneAsset)
		{
			POLY_CORE_WARN("Cannot instantiate scene asset - handle {} is invalid", sceneAssetHandle.Get());
			return Entity::None();
		}

		return InstantiateNode(pSceneAsset, pSceneAsset->GetRootNodeIndex(), parent);
	}

	Entity Scene::InstantiateNode(SceneAsset* pSceneAsset, uint32 nodeIndex, Entity parent)
	{
		const SceneAsset::Node& node = pSceneAsset->GetNode(nodeIndex);

		Entity entity                             = CreateEntity();
		entity.GetComponent<TransformComponent>() = TransformComponent{
		    .Translation = node.Translation,
		    .Scale       = node.Scale,
		    .Orientation = node.Orientation};

		if (parent != Entity::None())
			entity.SetParent(parent);

		// The node entity itself only carries a renderable when there's exactly one - extra
		// renderables on the same node (multi-primitive meshes) get their own identity-transform
		// child entity, since MeshAssetComponent/MaterialComponent are one-per-entity.
		for (size_t i = 0; i < node.Renderables.size(); i++)
		{
			const SceneAsset::Renderable& renderable = node.Renderables[i];
			Entity                        target     = (i == 0) ? entity : CreateEntity();

			if (i != 0)
				target.SetParent(entity);

			target.AddComponent<MeshAssetComponent>(pSceneAsset->GetMeshAsset(renderable.MeshIndex));
			target.AddComponent<MaterialComponent>(pSceneAsset->GetMaterialAsset(renderable.MaterialIndex));
		}

		for (uint32 childIndex : node.ChildrenIndices)
			InstantiateNode(pSceneAsset, childIndex, entity);

		return entity;
	}

	void Scene::Update()
	{
		if (m_Registry.storage<DirtyTag>().empty())
			return;

		if (m_pRenderScene)
			m_pRenderScene->Update();

		if (m_pSceneRenderBridge)
			m_pSceneRenderBridge->Update();

		m_Registry.clear<DirtyTag>();
	}

	void Scene::CreateRenderScene(RenderGraphProgram& program)
	{
		m_pRenderScene = CreateRef<RenderScene>(*this, program);
	}

	void Scene::CreateSceneRenderBridge(Ref<RenderProgramInstance> pProgramInstance)
	{
		m_pSceneRenderBridge = CreateRef<SceneRenderBridge>(*this, std::move(pProgramInstance));
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
} // namespace Poly