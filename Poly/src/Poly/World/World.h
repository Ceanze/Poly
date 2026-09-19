#pragma once

#include "Poly/Resources/AssetHandle.h"
#include "Poly/Scene/Entity.h"

#include <array>
#include <functional>

namespace Poly
{
	class SceneAsset;

	class World
	{
	public:
		enum class Phase : uint8
		{
			PreUpdate,
			Update,
			PostUpdate,
			Count
		};

		/**
		 * Creates an empty world
		 * @param name - name of the world
		 */
		World(const std::string_view name);
		~World();

		CLASS_REMOVE_COPY(World);

		/**
		 * Creates and adds an entity with transform, hierarchy and ID components to the world
		 * @return new entity
		 */
		Entity CreateEntity();

		/**
		 * Creates and adds an entity with transform, hierarchy and ID components to the world with a predetermined PolyID
		 * @param id - ID of the entity to create
		 * @return new entity
		 */
		Entity CreateEntity(PolyID id);

		/**
		 * Destroys a previously created entity
		 * @param entity - entity to destroy
		 */
		void DestroyEntity(Entity entity);

		/**
		 * Instantiates the node hierarchy of a scene asset into the world
		 * @param sceneAssetHandle - handle to the scene asset to instantiate
		 * @param parent - entity to parent the instantiated root to, Entity::None() to create it unparented
		 * @return root entity of the instantiated hierarchy, or Entity::None() if the handle is invalid
		 */
		Entity Instantiate(AssetHandle<SceneAsset> sceneAssetHandle, Entity parent = Entity::None());

		/**
		 * Sets the name of the world
		 * @param name - new name
		 */
		void SetName(std::string_view name);

		/**
		 * @return name of the world
		 */
		const std::string& GetName() const;

		/**
		 * @return true if the world contains no live entities
		 */
		bool IsEmpty() const;

		/**
		 * Runs all systems once. Phases run in order PreUpdate, Update, PostUpdate, and systems
		 * within a phase run in the order they were added
		 */
		void Update();

		/**
		 * Constructs and adds a class-based system. The world owns the system and keeps it alive for its own lifetime.
		 *
		 * Required, one of (Update is preferred if both exist):
		 * - void Update(World& world)
		 * - void operator()(World& world)
		 *
		 * Optional, detected automatically (no base class needed):
		 * - void OnInit(World& world)     - called once, immediately when the system is added
		 * - void OnShutdown(World& world) - called once when the world is destroyed, in reverse order of adding
		 *
		 * Example:
		 * @code
		 * class MySystem
		 * {
		 * public:
		 *     MySystem(float speed) : m_Speed(speed) {}
		 *     void OnInit(World& world) {}     // optional
		 *     void Update(World& world) {}     // or operator()(World&)
		 *     void OnShutdown(World& world) {} // optional
		 * private:
		 *     float m_Speed;
		 * };
		 *
		 * MySystem& system = world.AddSystem<MySystem>(World::Phase::Update, 2.0f);
		 * @endcode
		 *
		 * @tparam TSystem - system type
		 * @param phase - phase the system runs in
		 * @param args - arguments forwarded to the constructor of TSystem
		 * @return reference to the created system, valid for the lifetime of the world
		 */
		template<typename TSystem, typename... Args>
		TSystem& AddSystem(Phase phase, Args&&... args)
		{
			Ref<TSystem> pSystem = CreateRef<TSystem>(std::forward<Args>(args)...);

			SystemEntry entry;
			entry.Name = typeid(TSystem).name();

			if constexpr (requires(TSystem& system, World& world) { system.Update(world); })
			{
				entry.Update = [pSystem](World& world) {
					pSystem->Update(world);
				};
			}
			else
			{
				entry.Update = [pSystem](World& world) {
					(*pSystem)(world);
				};
			}

			if constexpr (requires(TSystem& system, World& world) { system.OnShutdown(world); })
			{
				entry.Shutdown = [pSystem](World& world) {
					pSystem->OnShutdown(world);
				};
			}

			if constexpr (requires(TSystem& system, World& world) { system.OnInit(world); })
				pSystem->OnInit(*this);

			m_Systems[static_cast<size_t>(phase)].push_back(std::move(entry));
			return *pSystem;
		}

		/**
		 * Adds a stateless system from any callable taking World&.
		 * Lifecycle hooks (OnInit/OnShutdown) are not available for callables, use the class-based
		 * AddSystem<TSystem> overload if the system needs them.
		 *
		 * Example:
		 * @code
		 * world.AddSystem("ClearDirty", World::Phase::PostUpdate, [](World& world) {});
		 * @endcode
		 *
		 * @param name - name of the system, used for debugging
		 * @param phase - phase the system runs in
		 * @param func - callable invoked with the world each update
		 */
		template<typename TFunc>
		void AddSystem(std::string_view name, Phase phase, TFunc&& func)
		{
			SystemEntry entry;
			entry.Name   = name;
			entry.Update = std::forward<TFunc>(func);
			m_Systems[static_cast<size_t>(phase)].push_back(std::move(entry));
		}

	private:
		struct SystemEntry
		{
			std::string                 Name;
			std::function<void(World&)> Update;
			std::function<void(World&)> Shutdown;
		};

		Entity InstantiateNode(SceneAsset* pSceneAsset, uint32 nodeIndex, Entity parent);

		entt::registry m_Registry;
		std::string    m_Name;

		std::array<std::vector<SystemEntry>, static_cast<size_t>(Phase::Count)> m_Systems;
	};
} // namespace Poly