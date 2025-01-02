#pragma once

#include <entt/entt.hpp>

#include "Poly/Rendering/RenderGraph/ResourceGroup.h"
#include "Poly/Model/Model.h" // TODO: See if this can be removed

namespace Poly
{
	class Entity;
	class RenderScene;
	class RenderGraphProgram;

	class Scene
	{
	public:
		static constexpr const char* VERTICES_RESOURCE_NAME = "vertices";
		static constexpr const char* INSTANCE_RESOURCE_NAME = "instance";
		static constexpr const char* MATERIAL_RESOURCE_NAME = "material";
		static constexpr const char* ALBEDO_TEX_RESOURCE_NAME = "albedoTex";
		static constexpr const char* NORMAL_TEX_RESOURCE_NAME = "normalTex";
		static constexpr const char* COMBINED_TEX_RESOURCE_NAME = "combinedTex";
		static constexpr const char* METALLIC_TEX_RESOURCE_NAME = "metallicTex";
		static constexpr const char* ROUGHNESS_TEX_RESOURCE_NAME = "roughnessTex";
		static constexpr const char* AO_TEX_RESOURCE_NAME = "aoTex";

		struct DrawData
		{
			MeshInstance	MeshInstance;
			uint32			InstanceCount;
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
		 * Gets the resource group for the scene with all of the resources it might use
		 * NOTE: Resource pointers might not be valid before render graph execution
		 * @return ResourceGroup const pointer
		 */
		const ResourceGroup& GetResourceGroup() const { return m_ResourceGroup; }

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

		/**
		* Internally creates a render scene bound to the program provided.
		* This is automatically done when a scene is set to a render graph program.
		*/
		void CreateRenderScene(RenderGraphProgram& program);

	private:
		friend class Entity;
		friend class SceneRenderer; // TODO: Remove when scene renderer uses the new RenderScene instead
		friend class RenderScene; // TODO: Will be removed when interface for views exist
		friend class SceneSerializer;
		friend class EntitySerializer;

		PolyID GetIdOfEntity(entt::entity entity);

		std::string m_Name;

		entt::registry m_Registry;
		ResourceGroup m_ResourceGroup;
		Ref<RenderScene> m_pRenderScene;
	};
}