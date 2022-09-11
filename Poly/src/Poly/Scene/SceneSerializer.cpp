#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Poly/Resources/IOManager.h"

#include "Entity.h"
#include "Scene.h"
#include "Components.h"
#include "EntitySerializer.h"


namespace Poly
{
	void SceneSerializer::Serialize(const std::string& path)
	{
		YAML::Emitter emitter;
		emitter << YAML::BeginMap;
		emitter << YAML::Key << "Scene" << YAML::Value << m_pScene->GetName();
		emitter << YAML::Key << "Entites" << YAML::Value << YAML::BeginSeq;


		EntitySerializer entitySerializer;
		m_pScene->m_Registry.each([&](auto eid)
		{
			Entity entity = { m_pScene.get(), eid };

			SerializeEntity(entitySerializer, emitter, entity);
		});

		emitter << YAML::EndSeq;
		emitter << YAML::EndMap;

		std::ofstream file(IOManager::GetScenesFolder() + path);
		file << emitter.c_str();
		file.close();
	}

	void SceneSerializer::Deserialize(const std::string& path)
	{
		const std::string fullPath = IOManager::GetScenesFolder() + path;
		if (!m_pScene->IsEmpty())
		{
			POLY_CORE_WARN("Cannot deserialize scene at path {}, active scene is not empty", fullPath);
			return;
		}

		if (!IOManager::FileExists(fullPath))
		{
			POLY_CORE_WARN("Cannot deserialize scene at path {}, file cannot be found", fullPath);
			return;
		}

		YAML::Node sceneFile = YAML::LoadFile(fullPath);

		bool isValid = !!sceneFile["Scene"];

		if (!isValid)
		{
			POLY_CORE_WARN("Cannot deserialize scene at path {}, scene file is not valid", fullPath);
			return;
		}

		m_pScene->SetName(sceneFile["Scene"].as<std::string>());

		EntitySerializer entitySerializer;
		if (auto entitiesNode = sceneFile["Entites"])
		{
			for (auto entityNode : entitiesNode)
				if (!DeserializeEntity(entitySerializer, entityNode))
				{
					POLY_CORE_WARN("Cannot deserialize scene at path {}, an entity in scene file is invalid", fullPath);
					return;
				}
		}
	}

	void SceneSerializer::SerializeEntity(EntitySerializer& entitySerializer, YAML::Emitter& emitter, Entity entity)
	{
		emitter << YAML::BeginMap;
		emitter << YAML::Key << "EntityID" << YAML::Value << entity.GetPolyID();

		entitySerializer.Serialize(emitter, entity);

		emitter << YAML::EndMap;
	}

	bool SceneSerializer::DeserializeEntity(EntitySerializer& entitySerializer, YAML::Node& node)
	{
		bool isValid = !!node["EntityID"];

		if (!isValid)
			return false;

		auto id = node["EntityID"].as<uint64>();
		Entity entity = m_pScene->GetOrCreateEntityWithID(id);

		entitySerializer.Deserialize(node, entity);

		return true;
	}
}

/**
 *
 * Scene: MyFirstScene
 * * PLACE FOR MORE METADATA *
 * Entites: [SEQ]
 * { [MAP IN SEQ]
 * 	- Entity: PolyID [KEY: VAL]
 * 	- Transform: [KEY: MAP]
 * 		{
 * 		 - Scale: Vec3
 * 		 - Rotation: Vec3
 * 		 - Translation: Vec3
 * 		}
 * 	 - Tag: [KEY: MAP]
 * 		{
 * 		 - Name: CoolComp1
 * 		}
 * },
 * { [MAP IN SEQ]
 * 	- Entity: PolyID
 * 	- Transform:
 * 		{
 * 		 - Scale: Vec3
 * 		 - Rotation: Vec3
 * 		 - Translation: Vec3
 * 		}
 * 	 - Tag:
 * 		{
 * 		 - Name: CoolComp2
 * 		}
 * }
 */