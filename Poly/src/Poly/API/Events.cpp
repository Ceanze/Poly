#include "Events.h"

namespace Poly::API
{
	EventBus::Subscription::~Subscription()
	{
		Reset();
	}

	EventBus::Subscription::Subscription(Subscription&& other) noexcept
	    : m_pBus(std::exchange(other.m_pBus, nullptr))
	    , m_ID(other.m_ID)
	{}

	EventBus::Subscription& EventBus::Subscription::operator=(Subscription&& other) noexcept
	{
		if (this != &other)
		{
			Reset();
			m_pBus = std::exchange(other.m_pBus, nullptr);
			m_ID   = other.m_ID;
		}

		return *this;
	}

	void EventBus::Subscription::Reset()
	{
		if (!m_pBus)
			return;

		std::erase_if(m_pBus->m_Listeners, [this](const ListenerEntry& entry) { return entry.ID == m_ID; });
		m_pBus = nullptr;
	}

	EventBus::Subscription EventBus::Subscribe(Listener listener)
	{
		const uint32 id = m_NextID++;
		m_Listeners.push_back({id, std::move(listener)});
		return Subscription(this, id);
	}

	void EventBus::Queue(ApiEvent event)
	{
		m_Queue.push_back(std::move(event));
	}

	void EventBus::Flush()
	{
		// Listeners are allowed to make changes that queue new events, those are delivered by the next flush
		const std::vector<ApiEvent> events = std::exchange(m_Queue, {});
		for (const ApiEvent& event : events)
		{
			for (size_t i = 0; i < m_Listeners.size(); i++)
				m_Listeners[i].Callback(event);
		}
	}
} // namespace Poly::API
