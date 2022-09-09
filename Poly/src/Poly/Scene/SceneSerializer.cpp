#include "SceneSerializer.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

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

		m_pScene->m_Registry.each([&](auto eid)
		{
			Entity entity = { m_pScene.get(), eid };

			SerializeEntity(emitter, entity);
		});

		emitter << YAML::EndSeq;
		emitter << YAML::EndMap;

		std::ofstream file(path);
		file << emitter.c_str();
		file.close();
	}

	void SceneSerializer::Deserialize(const std::string& path)
	{

	}

	void SceneSerializer::SerializeEntity(YAML::Emitter& emitter, Entity entity)
	{
		emitter << YAML::BeginMap;
		emitter << YAML::Key << "EntityID" << YAML::Value << entity.GetPolyID();

		EntitySerializer entitySerializer(emitter);
		entitySerializer.Serialize(entity);

		emitter << YAML::EndMap;
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