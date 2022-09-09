#include "EntitySerializer.h"

#include "Entity.h"
#include "Components.h"
#include "Poly/Model/Model.h"

#include <yaml-cpp/yaml.h>

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
		out << YAML::BeginSeq << q.x << q.y << q.z << q.z << YAML::EndSeq;
		return out;
	}

	EntitySerializer::EntitySerializer(YAML::Emitter& emitter)
	: m_Emitter(emitter) {}

	void EntitySerializer::Serialize(const Entity& entity)
	{
		if (entity.HasComponent<TransformComponent>())
			SerializeTransformComponent(entity);

		if (entity.HasComponent<HierarchyComponent>())
			SerializeHierarchyComponent(entity);

		if (entity.HasComponent<MeshComponent>())
			SerializeMeshComponent(entity);
	}

	void EntitySerializer::SerializeTransformComponent(const Entity& entity)
	{
		m_Emitter << YAML::Key << "TransformComponent";
		m_Emitter << YAML::BeginMap;

		auto& transformComp = entity.GetComponent<TransformComponent>();
		m_Emitter << YAML::Key << "Translation" << YAML::Value << transformComp.Translation;
		m_Emitter << YAML::Key << "Scale" << YAML::Value << transformComp.Scale;
		m_Emitter << YAML::Key << "Orientation" << YAML::Value << transformComp.Orientation;

		m_Emitter << YAML::EndMap;
	}

	void EntitySerializer::SerializeHierarchyComponent(const Entity& entity)
	{
		m_Emitter << YAML::Key << "HierarchyComponent";
		m_Emitter << YAML::BeginMap;

		auto& hierarchyComp = entity.GetComponent<HierarchyComponent>();
		Scene* pScene = entity.GetScene();
		m_Emitter << YAML::Key << "ChildrenCount" << YAML::Value << static_cast<int>(hierarchyComp.ChildrenCount);
		m_Emitter << YAML::Key << "Parent" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.Parent);
		m_Emitter << YAML::Key << "First" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.First);
		m_Emitter << YAML::Key << "Next" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.Next);
		m_Emitter << YAML::Key << "Previous" << YAML::Value << pScene->GetIdOfEntity(hierarchyComp.Previous);

		m_Emitter << YAML::EndMap;
	}

	void EntitySerializer::SerializeMeshComponent(const Entity& entity)
	{
		m_Emitter << YAML::Key << "MeshComponent";
		m_Emitter << YAML::BeginMap;

		auto& meshComp = entity.GetComponent<MeshComponent>();
		m_Emitter << YAML::Key << "Model" << YAML::Value << meshComp.pModel->GetPolyID();
		m_Emitter << YAML::Key << "MeshIndex" << YAML::Value << meshComp.MeshIndex;

		m_Emitter << YAML::EndMap;
	}
}