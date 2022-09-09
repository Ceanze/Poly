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
		EntitySerializer(YAML::Emitter& emitter);

		void Serialize(const Entity& entity);

	private:
		YAML::Emitter& m_Emitter;

		void SerializeTransformComponent(const Entity& entity);
		void SerializeHierarchyComponent(const Entity& entity);
		void SerializeMeshComponent(const Entity& entity);
	};

	class EntityDeserializer
	{
	public:
		EntityDeserializer(const YAML::Node& node);
	};
}