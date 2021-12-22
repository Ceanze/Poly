#pragma once

#include "ECSTypes.h"
#include "EntityRegistry.h"

namespace Poly
{
	uint32 Entity::GetID() const { return m_Id; }

	template<typename T>
	T& Entity::AddComponent()
	{
		return m_pRegistry->AddComponent<T>(*this);
	}

	template<typename T, typename... Args>
	T& Entity::AddComponent(Args&&... args)
	{
		return m_pRegistry->AddComponent<T>(*this, std::forward<Args>(args)...);
	}

	template<typename T>
	T& Entity::AddComponent(const T& comp)
	{
		return m_pRegistry->AddComponent(*this, comp);
	}

	template<typename T>
	T& Entity::AddComponent(T&& comp)
	{
		return m_pRegistry->AddComponent(*this, comp);
	}

	template<typename T>
	T& Entity::GetComponent()
	{
		return m_pRegistry->GetComponent<T>(*this);
	}

	template<typename T>
	bool Entity::HasComponent()
	{
		return m_pRegistry->HasComponent<T>(*this);
	}

	template<typename T>
	void Entity::RemoveComponent()
	{
		m_pRegistry->RemoveComponent<T>(*this);
	}

	Entity::Entity(const Entity& other) { m_Id = other.m_Id; m_pRegistry = other.m_pRegistry; }
	Entity& Entity::operator=(const Entity& other) { m_Id = other.m_Id; m_pRegistry = other.m_pRegistry; return *this; }
	bool Entity::operator==(const Entity& other) const { return m_Id == other.m_Id && m_pRegistry == other.m_pRegistry; }
	bool Entity::operator!=(const Entity& other) const { return !(*this == other); }
	Entity::operator uint32() const { return GetID(); }
}