#pragma once

#include "ECSTypes.h"

#define ARRAY_INIT_SIZE 64

namespace Poly
{
	class IComponentArray
	{
	public:
		IComponentArray() = default;
		virtual ~IComponentArray() = default;

		virtual bool HasComponent(EntityID id) = 0;

		virtual void RemoveComponent(EntityID id) = 0;
	};

	/**
	 * A densely packed vector/array containing one specific component type T
	 */
	template<typename T>
	class ComponentArray : public IComponentArray
	{
	public:
		ComponentArray() { m_Components.reserve(ARRAY_INIT_SIZE); }

		~ComponentArray() = default;

		static Unique<ComponentArray<T>> Create() { return CreateUnique<ComponentArray<T>>(); }

		template<typename... Args>
		T& CreateComponent(EntityID id, Args&&... args);

		T& CreateComponent(EntityID id);

		T& AddComponent(EntityID id, const T& comp);

		T& AddComponent(EntityID id, T&& comp);

		T& GetComponent(EntityID id);

		bool HasComponent(EntityID id) override final;

		void RemoveComponent(EntityID id) override final;

	private:
		std::unordered_map<EntityID, uint32> m_IDToIndex;
		std::vector<T> m_Components;
	};

	template<typename T>
	template<typename... Args>
	T& ComponentArray<T>::CreateComponent(EntityID id, Args&&... args)
	{
		POLY_VALIDATE(!m_IDToIndex.contains(id), "Cannot add component. Entity already has component type");
		uint32 index = m_Components.size();
		m_IDToIndex[id] = index;
		m_Components.push_back(T(std::forward(args)...));
		return m_Components[index];
	}

	template<typename T>
	T& ComponentArray<T>::CreateComponent(EntityID id)
	{
		POLY_VALIDATE(!m_IDToIndex.contains(id), "Cannot add component. Entity already has component type");
		uint32 index = m_Components.size();
		m_IDToIndex[id] = index;
		m_Components.push_back(T());
		return m_Components[index];
	}

	template<typename T>
	T& ComponentArray<T>::AddComponent(EntityID id, const T& comp)
	{
		POLY_VALIDATE(!m_IDToIndex.contains(id), "Cannot add component. Entity already has component type");
		uint32 index = m_Components.size();
		m_IDToIndex[id] = index;
		m_Components.push_back(comp);
		return m_Components[index];
	}

	template<typename T>
	T& ComponentArray<T>::AddComponent(EntityID id, T&& comp)
	{
		POLY_VALIDATE(!m_IDToIndex.contains(id), "Cannot add component. Entity already has component type");
		uint32 index = m_Components.size();
		m_IDToIndex[id] = index;
		m_Components.push_back(comp);
		return m_Components[index];
	}

	template<typename T>
	T& ComponentArray<T>::GetComponent(EntityID id)
	{
		POLY_VALIDATE(m_IDToIndex.contains(id), "Cannot get component. Entity does not have component type");
		uint32 index = m_IDToIndex[id];
		return m_Components[index];
	}

	template<typename T>
	bool ComponentArray<T>::HasComponent(EntityID id)
	{
		return m_IDToIndex.contains(id);
	}

	template<typename T>
	void ComponentArray<T>::RemoveComponent(EntityID id)
	{
		POLY_VALIDATE(m_IDToIndex.contains(id), "Cannot remove component. Entity does not have component type");
		uint32 index = m_IDToIndex[id];
		m_Components[index] = m_Components.back();
		m_Components.pop_back();
		m_IDToIndex.erase(id);
	}
}