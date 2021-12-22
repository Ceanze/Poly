#pragma once

#include <typeinfo>
#include <typeindex>
#include <queue>

#include "ECSTypes.h"
#include "ComponentArray.h"

namespace Poly
{
	class EntityRegistry
	{
	public:
		EntityRegistry() = default;
		~EntityRegistry() = default;

		Entity CreateEntity();

		void DestroyEntity(Entity entity);

		template<typename T>
		T& AddComponent(Entity entity);

		template<typename T, typename... Args>
		T& AddComponent(Entity entity, Args&&... args);

		template<typename T>
		T& AddComponent(Entity entity, const T& comp);

		template<typename T>
		T& AddComponent(Entity entity, T&& comp);

		template<typename T>
		T& GetComponent(Entity entity);

		template<typename T>
		bool HasComponent(Entity entity);

		template<typename T>
		void RemoveComponent(Entity entity);

	private:
		template<typename T>
		ComponentArray<T>* GetArray();

		std::unordered_map<std::type_index, Unique<IComponentArray>> m_Components;
		EntityID m_NextEntityID = 0;
		std::queue<EntityID> m_DeletedEntityIDs;
	};

	Entity EntityRegistry::CreateEntity()
	{
		if (!m_DeletedEntityIDs.empty())
		{
			EntityID id = m_DeletedEntityIDs.front();
			m_DeletedEntityIDs.pop();
			return Entity(id, this);
		}

		return Entity(m_NextEntityID++, this);
	}

	void EntityRegistry::DestroyEntity(Entity entity)
	{
		m_DeletedEntityIDs.push(entity);
		for (auto& arrPair : m_Components)
		{
			if (arrPair.second->HasComponent(entity))
				arrPair.second->RemoveComponent(entity);
		}
	}

	template<typename T>
	T& EntityRegistry::AddComponent(Entity entity)
	{
		POLY_VALIDATE(!HasComponent<T>(entity), "Cannot add component of entity {}. Entity already have that component type", entity.GetID());
		return GetArray<T>()->CreateComponent(entity);
	}

	template<typename T, typename... Args>
	T& EntityRegistry::AddComponent(Entity entity, Args&&... args)
	{
		POLY_VALIDATE(!HasComponent<T>(entity), "Cannot add component of entity {}. Entity already have that component type", entity.GetID());
		return GetArray<T>()->CreateComponent(entity, std::forward(args)...);
	}

	template<typename T>
	T& EntityRegistry::AddComponent(Entity entity, const T& comp)
	{
		POLY_VALIDATE(!HasComponent<T>(entity), "Cannot add component of entity {}. Entity already have that component type", entity.GetID());
		return GetArray<T>()->AddComponent(entity, comp);
	}

	template<typename T>
	T& EntityRegistry::AddComponent(Entity entity, T&& comp)
	{
		POLY_VALIDATE(!HasComponent<T>(entity), "Cannot add component of entity {}. Entity already have that component type", entity.GetID());
		return GetArray<T>()->AddComponent(entity, comp);
	}

	template<typename T>
	T& EntityRegistry::GetComponent(Entity entity)
	{
		POLY_VALIDATE(HasComponent<T>(entity), "Cannot get component of entity {}. Entity does not have that component type", entity.GetID());
		return GetArray<T>()->GetComponent(entity);
	}

	template<typename T>
	bool EntityRegistry::HasComponent(Entity entity)
	{
		if (!m_Components.contains(std::type_index(typeid(T))))
			return false;

		return GetArray<T>()->HasComponent(entity);
	}

	template<typename T>
	void EntityRegistry::RemoveComponent(Entity entity)
	{
		POLY_VALIDATE(HasComponent<T>(entity), "Cannot remove component of entity {}. Entity does not have that component type", entity.GetID());
		GetArray<T>()->RemoveComponent(entity);
	}

	template<typename T>
	ComponentArray<T>* EntityRegistry::GetArray()
	{
		std::type_index index(typeid(T));
		if (!m_Components.contains(index))
			m_Components[index] = CreateUnique<ComponentArray<T>>();
		return static_cast<ComponentArray<T>*>(m_Components[index].get());
	}
}