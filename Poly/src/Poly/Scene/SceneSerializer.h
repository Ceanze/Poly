#pragma once

#include "Poly/Scene/Entity.h"

namespace YAML
{
	class Emitter;
	class Node;
}

namespace Poly
{
	class Scene;

	class SceneSerializer
	{
	public:
		SceneSerializer(Ref<Scene> pScene) : m_pScene(pScene) {}
		~SceneSerializer() = default;

		void Serialize(const std::string& path);

		void Deserialize(const std::string& path);

	private:
		void SerializeEntity(EntitySerializer& entitySerializer, YAML::Emitter& emitter, Entity entity);
		bool DeserializeEntity(EntitySerializer& entitySerializer, YAML::Node& node);

		Ref<Scene> m_pScene;
	};
}