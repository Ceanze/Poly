#pragma once

#include "Event.h"
#include "MemberFunctionHandler.h"
#include <typeindex>
#include <unordered_map>
#include <vector>

/**
	Events uses the following structure:	bool funcName(EventType* evnt)
	Events return true if the event has been handled, false if not.

	Pointers to member functions act different than normal function pointers
	instead of 
		int (*)(char,float)
	we get
		int (Class::*)(char,float)
	for non static member functions.
	In order to call the correct member function we also need to know the instance
	of the class, hence the requirement for the "this" param.
**/

namespace Poly {

	typedef std::vector<HandlerFunctionBase*> HandlerList;

	class EventBus
	{
	public:
		EventBus() = default;
		~EventBus() = default;

		// Publish an event
		template<class EventType>
		void publish(EventType* e);

		// Subscribe to an event
		template<class T, class EventType>
		void subscribe(T* instance, bool (T::*func)(EventType*));

		// Unsubscribe to an event
		template<class T, class EventType>
		void unsubscribe(T* instance, bool (T::* func)(EventType*));

		static EventBus& get();

	private:
		std::unordered_map<std::type_index, HandlerList*> subscribers;
	};

	template<class EventType>
	inline void EventBus::publish(EventType* e)
	{
		HandlerList* subs = subscribers[typeid(EventType)];

		if (!subs)
			return;

		for (auto it : *subs)
		{
			if ((*it).exec(e))
				break;
		}
	}

	template<class T, class EventType>
	inline void EventBus::subscribe(T* instance, bool(T::* func)(EventType*))
	{
		// Save the instance, member function pointer and the event type

		HandlerList* subs = subscribers[typeid(EventType)];

		if (!subs)
		{
			subs = new HandlerList;
			subscribers[typeid(EventType)] = subs;
		}

		HandlerFunctionBase* data = new MemberFunctionHandler<T, EventType>(instance, func);
		subs->push_back(data);
	}


	template<class T, class EventType>
	inline void EventBus::unsubscribe(T* instance, bool (T::* func)(EventType*))
	{
		HandlerList* subs = subscribers[typeid(EventType)];

		if (!subs)
			return;

		unsigned ID = getID<T, EventType>(instance);
		for (auto it : subs)
		{
			if ((*it)->ID == ID)
				subs->erase(it);
		}
	}

	inline EventBus& EventBus::get()
	{
		static EventBus instance;
		return instance;
	}
}