#pragma once

namespace Poly
{
	typedef uint32 EntityID;

	class EntityRegistry;

	/**
	 * Entity is a class with templated functions which therefore needs to have the functions in the header file.
	 * It also needs access to EntityRegistry - which in turn has multiple headers which needs access to Entity.
	 * To avoid circular dependency this is a forward declaration of the Entity class, with the implementation in Entity.h
	 */
	class Entity
	{
	public:
		Entity() = default;
		~Entity() = default;

		uint32 GetID() const;

		template<typename T>
		T& AddComponent();

		template<typename T, typename... Args>
		T& AddComponent(Args&&... args);

		template<typename T>
		T& AddComponent(const T& comp);

		template<typename T>
		T& AddComponent(T&& comp);

		template<typename T>
		T& GetComponent();

		template<typename T>
		bool HasComponent();

		template<typename T>
		void RemoveComponent();

		Entity(const Entity& other);
		Entity& operator=(const Entity& other);
		bool operator==(const Entity& other) const;
		bool operator!=(const Entity& other) const;
		operator uint32() const;


	private:
		friend class EntityRegistry;

		Entity(uint32 id, EntityRegistry* pRegistry) : m_Id(id), m_pRegistry(pRegistry) {};

		uint32 m_Id = 0;
		EntityRegistry* m_pRegistry = nullptr;
	};
}