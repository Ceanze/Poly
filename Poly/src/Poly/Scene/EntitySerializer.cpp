#include "EntitySerializer.h"

#include "Entity.h"
#include "Components.h"

#include "Poly/Model/Model.h"

#include "Poly/Resources/ResourceManager.h"

#include <yaml-cpp/yaml.h>

namespace YAML {

	template<>
	struct convert<glm::vec2>
	{
		static Node encode(const glm::vec2& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec2& rhs)
		{
			if (!node.IsSequence() || node.size() != 2)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec3>
	{
		static Node encode(const glm::vec3& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec3& rhs)
		{
			if (!node.IsSequence() || node.size() != 3)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::vec4>
	{
		static Node encode(const glm::vec4& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::vec4& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};

	template<>
	struct convert<glm::quat>
	{
		static Node encode(const glm::quat& rhs)
		{
			Node node;
			node.push_back(rhs.x);
			node.push_back(rhs.y);
			node.push_back(rhs.z);
			node.push_back(rhs.w);
			node.SetStyle(EmitterStyle::Flow);
			return node;
		}

		static bool decode(const Node& node, glm::quat& rhs)
		{
			if (!node.IsSequence() || node.size() != 4)
				return false;

			rhs.x = node[0].as<float>();
			rhs.y = node[1].as<float>();
			rhs.z = node[2].as<float>();
			rhs.w = node[3].as<float>();
			return true;
		}
	};
}

namespace Poly
{
	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
		return out;
	}

	YAML::Emitter& operator<<(YAML::Emitter& out, const glm::quat& q)
	{
		out << YAML::Flow;
		out << YAML::BeginSeq << q.x << q.y << q.z << q.w << YAML::EndSeq;
		return out;
	}

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

		auto& hierarchyComp = entity.GetComponent<HierarchyComponent>();
		Scene* pScene = entity.GetScene();
		emitter << YAML::Key << "ChildrenCount" << YAML::Value << static_cast<int>(hierarchyComp.ChildrenCount);
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
		transformComp.Scale = node["Scale"].as<glm::vec3>();
		transformComp.Orientation = node["Orientation"].as<glm::quat>();
	}

	void EntitySerializer::DeserializeHierarchyComponent(YAML::Node& node, Entity& entity)
	{
		// Entity should always have hierarchy component
		HierarchyComponent& hierarchyComp = entity.GetComponent<HierarchyComponent>();

		hierarchyComp.ChildrenCount = static_cast<uint8>(node["ChildrenCount"].as<int>());
		hierarchyComp.Parent = entity.GetScene()->GetOrCreateEntityWithID(node["Parent"].as<uint64>());
		hierarchyComp.First = entity.GetScene()->GetOrCreateEntityWithID(node["First"].as<uint64>());
		hierarchyComp.Next = entity.GetScene()->GetOrCreateEntityWithID(node["Next"].as<uint64>());
		hierarchyComp.Previous = entity.GetScene()->GetOrCreateEntityWithID(node["Previous"].as<uint64>());
	}

	void EntitySerializer::DeserializeMeshComponent(YAML::Node& node, Entity& entity)
	{
		if (entity.HasComponent<MeshComponent>())
		{
			POLY_CORE_WARN("Cannot deserialize mesh component, entity already has component");
			return;
		}

		PolyID modelID = PolyID(node["Model"].as<uint64>());
		uint32 meshIndex = node["MeshIndex"].as<uint32>();

		Model* pModel;
		if (ResourceManager::IsResourceLoaded(modelID))
			pModel = ResourceManager::GetModel(modelID);
		else
		{
			ResourceManager::LoadModel(modelID);
			pModel = ResourceManager::GetModel(modelID);
		}

		entity.AddComponent<MeshComponent>(pModel, meshIndex);
	}
}