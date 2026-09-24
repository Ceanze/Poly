#pragma once

#include "Poly/World/World.h"

#include <functional>

namespace YAML
{
	class Emitter;
	class Node;
} // namespace YAML

namespace Poly
{
	/*
	 * Saves and loads the entities of a world, together with their components, as yaml (.polyworld).
	 *
	 * Only the data of a world is serialized. Systems and derived runtime state (WorldTransformComponent, DirtyTag, ...)
	 * are not, they are recreated by the world itself.
	 *
	 * Components are opt-in, only components registered with RegisterComponent() are serialized. The built-in data
	 * components are registered on construction.
	 *
	 * Usage:
	 * @code
	 * WorldSerializer serializer;
	 * serializer.RegisterComponent<HealthComponent>("Health",
	 *     [](YAML::Emitter& out, const HealthComponent& health) { out << YAML::Key << "Value" << YAML::Value << health.Value; },
	 *     [](const YAML::Node& node, HealthComponent& health) { health.Value = node["Value"].as<float>(); return true; });
	 *
	 * serializer.Save(world, "assets/worlds/MyWorld.polyworld");
	 * serializer.Load(otherWorld, "assets/worlds/MyWorld.polyworld");
	 * @endcode
	 */
	class WorldSerializer
	{
	public:
		static constexpr uint32 FORMAT_VERSION = 1;

		/**
		 * Writes the fields of the component, the surrounding yaml map is handled by the serializer
		 */
		template<typename TComponent>
		using SerializeFn = std::function<void(YAML::Emitter& out, const TComponent& component)>;

		/**
		 * Reads the fields of the component. The component is pre-filled with the entity's current
		 * component if it has one, else it is default constructed, so missing fields can be left as is
		 * @return false if the node is invalid, the component is then not added to the entity
		 */
		template<typename TComponent>
		using DeserializeFn = std::function<bool(const YAML::Node& node, TComponent& component)>;

		WorldSerializer();
		~WorldSerializer() = default;

		/**
		 * Registers a component to be serialized
		 * @tparam TComponent - component type, must be default constructible
		 * @param name - name of the component in the file, must be unique and stable across versions
		 * @param serialize - writes the component
		 * @param deserialize - reads the component
		 */
		template<typename TComponent>
		void RegisterComponent(std::string_view name, SerializeFn<TComponent> serialize, DeserializeFn<TComponent> deserialize)
		{
			if (FindComponent(name))
			{
				POLY_CORE_WARN("Cannot register component '{}' for serialization, a component with that name is already registered", name);
				return;
			}

			ComponentEntry entry;
			entry.Name = name;

			entry.Has = [](const World& world, entt::entity entity) {
				return world.View<TComponent>().contains(entity);
			};

			entry.Serialize = [serialize = std::move(serialize)](YAML::Emitter& out, const World& world, entt::entity entity) {
				serialize(out, world.View<TComponent>().template get<TComponent>(entity));
			};

			entry.Deserialize = [deserialize = std::move(deserialize)](const YAML::Node& node, Entity& entity) {
				const bool hasComponent = entity.HasComponent<TComponent>();
				TComponent component    = hasComponent ? entity.GetComponent<TComponent>() : TComponent{};
				if (!deserialize(node, component))
					return false;

				if (hasComponent)
					entity.GetComponent<TComponent>() = std::move(component);
				else
					entity.AddComponent<TComponent>(std::move(component));

				return true;
			};

			m_Components.push_back(std::move(entry));
		}

		/**
		 * Saves all entities of the world with their registered components
		 * @param world - world to save
		 * @param vfsPath - VFS path of the file to write
		 * @return true if the file was written
		 */
		bool Save(const World& world, std::string_view vfsPath) const;

		/**
		 * Loads the entities of a file into the world. The world must not contain any entities.
		 * Assets referenced by components are loaded if they are not already
		 * @param world - world to load into, must be empty
		 * @param vfsPath - VFS path of the file to read
		 * @return true if the file was loaded, false if it could not be read or is invalid - the world is then left untouched
		 */
		bool Load(World& world, std::string_view vfsPath) const;

	private:
		struct ComponentEntry
		{
			std::string                                                     Name;
			std::function<bool(const World&, entt::entity)>                 Has;
			std::function<void(YAML::Emitter&, const World&, entt::entity)> Serialize;
			std::function<bool(const YAML::Node&, Entity&)>                 Deserialize;
		};

		void                  RegisterBuiltInComponents();
		const ComponentEntry* FindComponent(std::string_view name) const;
		void                  SerializeEntity(YAML::Emitter& out, const World& world, entt::entity entity) const;

		std::vector<ComponentEntry> m_Components;
	};
} // namespace Poly
