#pragma once

#include <functional>
#include <variant>

namespace Poly::API
{
	// A destroyed entity takes its descendants with it, only the root of the destroyed subtree is reported.
	// The same goes for the created event of a subtree that is restored by an undo
	struct EntityCreated
	{
		PolyID ID;
	};

	struct EntityDestroyed
	{
		PolyID ID;
	};

	struct EntityReparented
	{
		PolyID ID;
		PolyID OldParent;
		PolyID NewParent;
	};

	struct ComponentAdded
	{
		PolyID      ID;
		std::string Component;
	};

	struct ComponentRemoved
	{
		PolyID      ID;
		std::string Component;
	};

	struct FieldChanged
	{
		PolyID      ID;
		std::string Component;
		std::string Field;
	};

	struct WorldLoaded
	{
		std::string Name;
		std::string Path;
	};

	struct WorldCleared
	{
		std::string Name;
	};

	struct WorldSaved
	{
		std::string Path;
	};

	struct HistoryChanged
	{
		bool CanUndo = false;
		bool CanRedo = false;
		bool Dirty   = false;
	};

	using ApiEvent = std::variant<EntityCreated, EntityDestroyed, EntityReparented, ComponentAdded, ComponentRemoved, FieldChanged, WorldLoaded, WorldCleared, WorldSaved, HistoryChanged>;

	/*
	 * Queue of the changes made through the API. Events are not delivered when they happen, they are queued and
	 * delivered together by Flush() (EngineContext::Tick()), so a listener never sees a half applied change.
	 *
	 * Usage:
	 * @code
	 * EventBus::Subscription subscription = context.Events().Subscribe([](const ApiEvent& event) {
	 *     if (const EntityDestroyed* pDestroyed = std::get_if<EntityDestroyed>(&event)) {}
	 * });
	 * @endcode
	 */
	class EventBus
	{
	public:
		using Listener = std::function<void(const ApiEvent&)>;

		/**
		 * Keeps a listener subscribed for as long as it is alive. Must not outlive the EventBus
		 */
		class Subscription
		{
		public:
			Subscription() = default;
			~Subscription();

			Subscription(Subscription&& other) noexcept;
			Subscription& operator=(Subscription&& other) noexcept;
			CLASS_REMOVE_COPY(Subscription);

		private:
			friend class EventBus;

			Subscription(EventBus* pBus, uint32 id)
			    : m_pBus(pBus)
			    , m_ID(id)
			{}

			void Reset();

			EventBus* m_pBus = nullptr;
			uint32    m_ID   = 0;
		};

		EventBus()  = default;
		~EventBus() = default;
		CLASS_REMOVE_COPY(EventBus);

		/**
		 * @param listener - called for every event during Flush()
		 * @return subscription, the listener is removed when it is destroyed
		 */
		[[nodiscard]] Subscription Subscribe(Listener listener);

		/**
		 * Queues an event to be delivered by the next Flush()
		 */
		void Queue(ApiEvent event);

		/**
		 * Delivers all queued events in the order they were queued
		 */
		void Flush();

	private:
		struct ListenerEntry
		{
			uint32   ID;
			Listener Callback;
		};

		std::vector<ListenerEntry> m_Listeners;
		std::vector<ApiEvent>      m_Queue;
		uint32                     m_NextID = 1;
	};
} // namespace Poly::API
