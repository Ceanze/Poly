#pragma once

namespace YAML
{
	class Node;
	class Emitter;
}

namespace Poly
{
	class Entity;

	class EntitySerializer
	{
	public:
		EntitySerializer() = default;
		~EntitySerializer() = default;

		void Serialize(YAML::Emitter& emitter, const Entity& entity);

		void Deserialize(YAML::Node& node, Entity& entity);

	private:
		void SerializeTransformComponent(YAML::Emitter& emitter, const Entity& entity);
		void SerializeHierarchyComponent(YAML::Emitter& emitter, const Entity& entity);
		void SerializeMeshComponent(YAML::Emitter& emitter, const Entity& entity);

		void DeserializeTransformComponent(YAML::Node& node, Entity& entity);
		void DeserializeHierarchyComponent(YAML::Node& node, Entity& entity);
		void DeserializeMeshComponent(YAML::Node& node, Entity& entity);
	};
}