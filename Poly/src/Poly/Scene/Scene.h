#pragma once

#include "Poly/Model/Model.h" // TODO: See if this can be removed
#include "Poly/Resources/AssetHandle.h"

#include <entt/entt.hpp>

namespace Poly
{
	class SceneAsset;
	class Entity;

	class Scene
	{
	public:
		static constexpr const char* VERTICES_RESOURCE_NAME      = "vertices";
		static constexpr const char* INSTANCE_RESOURCE_NAME      = "instance";
		static constexpr const char* MATERIAL_RESOURCE_NAME      = "material";
		static constexpr const char* ALBEDO_TEX_RESOURCE_NAME    = "albedoTex";
		static constexpr const char* NORMAL_TEX_RESOURCE_NAME    = "normalTex";
		static constexpr const char* COMBINED_TEX_RESOURCE_NAME  = "combinedTex";
		static constexpr const char* METALLIC_TEX_RESOURCE_NAME  = "metallicTex";
		static constexpr const char* ROUGHNESS_TEX_RESOURCE_NAME = "roughnessTex";
		static constexpr const char* AO_TEX_RESOURCE_NAME        = "aoTex";

		struct DrawData
		{
			MeshInstance MeshInstance;
			uint32       InstanceCount;
		};

	public:
		Scene(const std::string& name);
		~Scene() = default;

		static Ref<Scene> Create(const std::string& name = "Untitled") { return CreateRef<Scene>(name); }

		/**
		 * Creates and adds an entity with transform component to the scene
		 * @return new entity
		 */
		Entity CreateEntity();

		/**
		 * Creates and adds an entity with transform component to the scene with a predetermined PolyID
		 * @param id - ID of the entity to create
		 * @return new entity
		 */
		Entity CreateEntityWithID(PolyID id);

		/**
		 * Gets an entity with the corresponding id. If no entity with that id exist, creates a new one with that id
		 * @param id - ID of the entity to get or create
		 * @return requested existing entity or new
		 */
		Entity GetOrCreateEntityWithID(PolyID id);

		/**
		 * Destroys a previously created entity
		 * @param entity - entity to destroy
		 */
		void DestroyEntity(Entity entity);

		/**
		 * Instantiates a loaded SceneAsset's node hierarchy into this scene as entities, wiring up
		 * TransformComponent/HierarchyComponent and, per renderable, MeshAssetComponent/MaterialComponent.
		 * @param sceneAssetHandle - handle to an already-loaded SceneAsset (see AssetHandler::Load<SceneAsset>)
		 * @param parent - optional entity to parent the instantiated hierarchy's root under
		 * @return the created root entity, else Entity::None() if the handle was invalid
		 */
		Entity InstantiateSceneAsset(AssetHandle<SceneAsset> sceneAssetHandle, Entity parent);

		/**
		 * Sets the name of the scene
		 * @param name - new name of the scene
		 */
		void SetName(const std::string& name) { m_Name = name; }

		/**
		 * @return the name of the scene - "Untitled" if no name has been set
		 */
		const std::string& GetName() const { return m_Name; }

		/**
		 * @return true if no entites are registered, false otherwise
		 */
		bool IsEmpty() const { return m_Registry.storage<entt::entity>()->empty(); }

		/**
		 * Updates the scene with the current entities
		 */
		void Update();

	private:
		friend class Entity;
		friend class SceneSerializer;
		friend class EntitySerializer;

		PolyID GetIdOfEntity(entt::entity entity);
		Entity InstantiateNode(SceneAsset* pSceneAsset, uint32 nodeIndex, Entity parent);

		std::string m_Name;

		entt::registry m_Registry;
	};
} // namespace Poly