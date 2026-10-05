#pragma once

#include "Poly/Reflection/ComponentDesc.h"

namespace Poly
{
	class FieldVisitor;

	/*
	 * Registry of all component types that can be serialized and accessed by name.
	 *
	 * Components are opt-in, see RegisterComponent<TComponent>() in ComponentRegistration.h. The built-in data components
	 * are registered by the engine. Derived runtime state (WorldTransformComponent, HierarchyComponent, DirtyTag, ...)
	 * is not registered, it is recreated by the world itself.
	 */
	class ComponentRegistry
	{
	public:
		CLASS_STATIC(ComponentRegistry);

		/**
		 * Registers the built-in components, done by Engine::Init(). Does nothing if already called
		 */
		static void RegisterBuiltInComponents();

		/**
		 * Adds a component description, prefer RegisterComponent<TComponent>() over calling this directly
		 * @return false if a component with the same name is already registered
		 */
		static bool Add(ComponentDesc desc);

		/**
		 * @return the component with the given name, nullptr if no such component is registered. Valid until shutdown
		 */
		static const ComponentDesc* Find(std::string_view name);

		/**
		 * @return all registered components in the order they were registered
		 */
		static const std::vector<Unique<ComponentDesc>>& GetAll();

		/**
		 * @return comma separated names of all registered components, for error messages
		 */
		static std::string GetNames();

	private:
		inline static std::vector<Unique<ComponentDesc>> s_Components;
		inline static bool                               s_BuiltInRegistered = false;
	};
} // namespace Poly
