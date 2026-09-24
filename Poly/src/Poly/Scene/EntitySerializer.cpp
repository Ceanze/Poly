#include "EntitySerializer.h"

#include "Components.h"
#include "Entity.h"
#include "Poly/Core/Serialization/YamlConversions.h"
#include "Poly/Model/Model.h"
#include "Poly/Resources/AssetManager.h"
#include "Scene.h"

#include <yaml-cpp/yaml.h>

namespace Poly
{
	void EntitySerializer::Serialize(YAML::Emitter& emitter, const Entity& entity)
	{
		if (entity.HasComponent<TransformComponent>())
			SerializeTransformComponent(emitter, entity);

		if (entity.HasComponent<HierarchyComponent>())
			SerializeHierarchyComponent(emitter, entity);

		if (entity.HasComponent<MeshComponent>())
			SerializeMeshComponent(emitter, entity);
	}

	void EntitySerializer::Deserialize(YAML::Node& node, Entity& entity)
	{
		if (auto transformNode = node["TransformComponent"])
			DeserializeTransformComponent(transformNode, entity);

		if (auto hierarchyNode = node["HierarchyComponent"])
			DeserializeHierarchyComponent(hierarchyNode, entity);

		if (auto meshNode = node["MeshComponent"])
			DeserializeMeshComponent(meshNode, entity);
	}

	void EntitySerializer::SerializeTransformComponent(YAML::Emitter& emitter, const Entity& entity)
	{
		emitter << YAML::Key << "TransformComponent";
		emitter << YAML::BeginMap;

		auto& transformComp = entity.GetComponent<TransformComponent>();
		emitter << YAML::Key << "Translation" << YAML::Value << transformComp.Translation;
		emitter << YAML::Key << "Scale" << YAML::Value << transformComp.Scale;
		emitter << YAML::Key << "Orientation" << YAML::Value << transformComp.Orientation;

		emitter << YAML::EndMap;
	}

	void EntitySerializer::SerializeHierarchyComponent(YAML::Emitter& emitter, const Entity& entity)
	{
		emitter << YAML::Key << "HierarchyComponent";
		emitter << YAML::BeginMap;

		auto&  hierarchyComp = entity.GetComponent<HierarchyComponent>();
		Scene* pScene        = entity.GetScene();
		emitter << YAML::Key << "ChildrenCount" << YAML::Value << hierarchyComp.ChildrenCount;
		emitter << YAML::Key << "Parent" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.Parent);
		emitter << YAML::Key << "First" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.First);
		emitter << YAML::Key << "Next" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.Next);
		emitter << YAML::Key << "Previous" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.Previous);

		emitter << YAML::EndMap;
	}

	void EntitySerializer::SerializeMeshComponent(YAML::Emitter& emitter, const Entity& entity)
	{
		emitter << YAML::Key << "MeshComponent";
		emitter << YAML::BeginMap;

		auto& meshComp = entity.GetComponent<MeshComponent>();
		emitter << YAML::Key << "Model" << YAML::Value << meshComp.pModel->GetPolyID();
		emitter << YAML::Key << "MeshIndex" << YAML::Value << meshComp.MeshIndex;

		emitter << YAML::EndMap;
	}

	void EntitySerializer::DeserializeTransformComponent(YAML::Node& node, Entity& entity)
	{
		// Entity should always have transform component
		TransformComponent& transformComp = entity.GetComponent<TransformComponent>();

		transformComp.Translation = node["Translation"].as<glm::vec3>();
		transformComp.Scale       = node["Scale"].as<glm::vec3>();
		transformComp.Orientation = node["Orientation"].as<glm::quat>();
	}

	void EntitySerializer::DeserializeHierarchyComponent(YAML::Node& node, Entity& entity)
	{
		// Entity should always have hierarchy component
		HierarchyComponent& hierarchyComp = entity.GetComponent<HierarchyComponent>();

		hierarchyComp.ChildrenCount = node["ChildrenCount"].as<uint32>();
		hierarchyComp.Parent        = entity.GetScene()->GetOrCreateEntityWithID(node["Parent"].as<uint64>());
		hierarchyComp.First         = entity.GetScene()->GetOrCreateEntityWithID(node["First"].as<uint64>());
		hierarchyComp.Next          = entity.GetScene()->GetOrCreateEntityWithID(node["Next"].as<uint64>());
		hierarchyComp.Previous      = entity.GetScene()->GetOrCreateEntityWithID(node["Previous"].as<uint64>());
	}

	void EntitySerializer::DeserializeMeshComponent(YAML::Node& node, Entity& entity)
	{
		if (entity.HasComponent<MeshComponent>())
		{
			POLY_CORE_WARN("Cannot deserialize mesh component, entity already has component");
			return;
		}

		PolyID modelID   = PolyID(node["Model"].as<uint64>());
		uint32 meshIndex = node["MeshIndex"].as<uint32>();

		Model* pModel;
		if (AssetManager::IsResourceLoaded(modelID))
			pModel = AssetManager::GetModel(modelID);
		else
		{
			AssetManager::LoadModel(modelID);
			pModel = AssetManager::GetModel(modelID);
		}

		entity.AddComponent<MeshComponent>(pModel, meshIndex);
	}
} // namespace Poly