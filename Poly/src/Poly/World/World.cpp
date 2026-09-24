#include "World.h"

#include "Poly/Resources/AssetHandler.h"
#include "Poly/Resources/AssetTypes/SceneAsset.h"
#include "Poly/Scene/Components/MaterialComponent.h"
#include "Poly/Scene/Components/MeshAssetComponent.h"
#include "Poly/Scene/Components/NameComponent.h"
#include "Poly/World/Systems/TransformSystem.h"

namespace Poly
{
	World::World(const std::string_view name)
	    : m_Name(name)
	{
		AddSystem<TransformSystem>(Phase::PostUpdate);
	}

	World::~World()
	{
		for (size_t phase = m_Systems.size(); phase-- > 0;)
		{
			std::vector<SystemEntry>& systems = m_Systems[phase];
			for (size_t i = systems.size(); i-- > 0;)
			{
				if (systems[i].Shutdown)
					systems[i].Shutdown(*this);
			}
		}
	}

	Entity World::CreateEntity()
	{
		return CreateEntity(PolyID());
	}

	Entity World::CreateEntity(PolyID id)
	{
		entt::entity entity = m_Registry.create();

		m_Registry.emplace<TransformComponent>(entity);
		m_Registry.emplace<WorldTransformComponent>(entity);
		m_Registry.emplace<HierarchyComponent>(entity);
		m_Registry.emplace<IDComponent>(entity, id);
		m_Registry.emplace<DirtyTag>(entity);

		return Entity({m_Registry, entity});
	}

	void World::DestroyEntity(Entity entity)
	{
		m_Registry.destroy(entity);
	}

	Entity World::GetEntity(entt::entity entity)
	{
		return Entity({m_Registry, entity});
	}

	Entity World::Instantiate(AssetHandle<SceneAsset> sceneAssetHandle, Entity parent)
	{
		SceneAsset* pSceneAsset = AssetHandler::Resolve(sceneAssetHandle);
		if (!pSceneAsset)
		{
			POLY_CORE_WARN("Cannot instantiate scene asset - handle {} is invalid", sceneAssetHandle.Get());
			return Entity::None();
		}

		return InstantiateNode(pSceneAsset, pSceneAsset->GetRootNodeIndex(), parent);
	}

	void World::SetName(std::string_view name)
	{
		m_Name = name;
	}

	const std::string& World::GetName() const
	{
		return m_Name;
	}

	bool World::IsEmpty() const
	{
		return m_Registry.storage<entt::entity>()->in_use() == 0;
	}

	void World::Update()
	{
		for (std::vector<SystemEntry>& systems : m_Systems)
		{
			for (SystemEntry& system : systems)
				system.Update(*this);
		}

		m_Registry.clear<DirtyTag>();
	}

	Entity World::InstantiateNode(SceneAsset* pSceneAsset, uint32 nodeIndex, Entity parent)
	{
		const SceneAsset::Node& node = pSceneAsset->GetNode(nodeIndex);

		Entity entity                             = CreateEntity();
		entity.GetComponent<TransformComponent>() = TransformComponent{
		    .Translation = node.Translation,
		    .Scale       = node.Scale,
		    .Orientation = node.Orientation};

		if (!node.Name.empty())
			entity.AddComponent<NameComponent>(node.Name);

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
} // namespace Poly
