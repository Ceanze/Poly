#pragma once

#include "Poly/Core/Result.h"
#include "Poly/World/World.h"

namespace Poly
{
	/**
	 * The data of an entity, detached from any world
	 */
	struct EntityData
	{
		PolyID                                           ID     = PolyID::None();
		PolyID                                           Parent = PolyID::None();
		std::vector<std::pair<std::string, std::string>> Components; // Component name and its fields as json
	};

	/**
	 * The data of a world, detached from any world. Parents are listed before their children and siblings in order
	 */
	struct WorldData
	{
		std::string             Name;
		std::vector<EntityData> Entities;
	};

	/*
	 * Saves and loads the entities of a world, together with their components, as yaml (.polyworld).
	 *
	 * Only the data of a world is serialized. Systems and derived runtime state (WorldTransformComponent, DirtyTag, ...)
	 * are not, they are recreated by the world itself.
	 *
	 * Components are opt-in, only components registered in the ComponentRegistry are serialized.
	 *
	 * Usage:
	 * @code
	 * RegisterComponent<HealthComponent>("Health");
	 *
	 * WorldSerializer::Save(world, "assets/worlds/MyWorld.polyworld");
	 * WorldSerializer::Load(otherWorld, "assets/worlds/MyWorld.polyworld");
	 * @endcode
	 */
	class WorldSerializer
	{
	public:
		static constexpr uint32 FORMAT_VERSION = 1;

		/**
		 * Saves all entities of the world with their registered components
		 * @param world - world to save
		 * @param vfsPath - VFS path of the file to write
		 * @return true if the file was written
		 */
		static bool Save(const World& world, std::string_view vfsPath);

		/**
		 * Loads the entities of a file into the world. The world must not contain any entities.
		 * Assets referenced by components are loaded if they are not already
		 * @param world - world to load into, must be empty
		 * @param vfsPath - VFS path of the file to read
		 * @return true if the file was loaded, false if it could not be read or is invalid - the world is then left untouched
		 */
		static bool Load(World& world, std::string_view vfsPath);

		/**
		 * Reads and validates a file without touching any world, see Apply()
		 * @param vfsPath - VFS path of the file to read
		 * @return the data of the file, or why it could not be read
		 */
		static Result<WorldData> Read(std::string_view vfsPath);

		/**
		 * Creates the entities of previously read data in the world and names the world after it.
		 * Components that are unknown or invalid are skipped with a warning
		 * @param world - world to create the entities in, must not have entities with the same IDs
		 * @param data - data from Read()
		 */
		static void Apply(World& world, const WorldData& data);

		/**
		 * Captures the ID, parent and registered components of an entity
		 * @param entity - entity to capture
		 * @return the data of the entity
		 */
		static EntityData CaptureEntity(Entity entity);

		/**
		 * Adds and fills the components of previously captured or read data on an entity.
		 * Components that are unknown or invalid are skipped with a warning
		 * @param entity - entity to apply the components to
		 * @param data - data of the entity
		 */
		static void ApplyComponents(Entity entity, const EntityData& data);
	};
} // namespace Poly
