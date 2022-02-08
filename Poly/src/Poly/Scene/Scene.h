#pragma once

#include <entt/entt.hpp>
#include "Poly/Rendering/RenderGraph/ResourceGroup.h"

namespace Poly
{
	class Entity;

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

	public:
		Scene();
		~Scene() = default;

		static Ref<Scene> Create() { return CreateRef<Scene>(); }

		/**
		 * Creates and adds an entity with transform component to the scene
		 * @return new entity
		 */
		Entity CreateEntity();

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
		const ResourceGroup* GetResourceGroup() const { return &m_ResourceGroup; }


	private:
		friend class Entity;
		friend class SceneRenderer;

		entt::registry m_Registry;
		ResourceGroup m_ResourceGroup;
	};
}