#pragma once

#include "Poly/Scene/Entity.h"

namespace YAML
{
	class Emitter;
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
		void SerializeEntity(YAML::Emitter& emitter, Entity entity);

		Ref<Scene> m_pScene;
	};
}