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

#define POLY_EVENT_SUB(className, funcName) EventBus::get().subscribe(this, &className::funcName);

#define POLY_EVENT_UNSUB(className, funcName) EventBus::get().unsubscribe(this, &className::funcName);

/* ev = constructed event to be sent */
#define POLY_EVENT_PUB(ev) EventBus::get().publish(&ev);

namespace Poly {

	typedef std::vector<HandlerFunctionBase*> HandlerList;

	class EventBus
	{
	public:
		EventBus() = default;
		~EventBus();

		// Publish an event
		template<class EventType>
		void publish(EventType* e);

		// Subscribe to an event
		template<class T, class EventType>
		void subscribe(T* instance, void (T::*func)(EventType*));

		// Unsubscribe to an event
		template<class T, class EventType>
		void unsubscribe(T* instance, void (T::* func)(EventType*));

		// Unsubscribe all events
		void unsubscribeAll();

		static EventBus& get();

	private:
		std::unordered_map<std::type_index, HandlerList*> subscribers;
	};

	inline EventBus::~EventBus()
	{
		unsubscribeAll();
	}

	template<class EventType>
	inline void EventBus::publish(EventType* e)
	{
		HandlerList* subs = subscribers[typeid(EventType)];

		if (!subs)
			return;

		for (auto it : *subs)
		{
			(*it).exec(e);
		}
	}

	template<class T, class EventType>
	inline void EventBus::subscribe(T* instance, void(T::* func)(EventType*))
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
	inline void EventBus::unsubscribe(T* instance, void (T::* func)(EventType*))
	{
		if (this->subscribers.empty())
			return;
		if (this->subscribers.find(typeid(EventType)) != this->subscribers.end())
		{
			HandlerList* handlers = this->subscribers[typeid(EventType)];

			unsigned id = getID<T, EventType>(instance);
			std::vector<HandlerFunctionBase*>::iterator it;
			for (it = handlers->begin(); it != handlers->end(); ++it)
			{
				if ((*it) == nullptr)
					continue;
				if ((*it)->ID == id)
				{
					delete* it;
					(*it) = nullptr;
					break;
				}
			}
		}
	}

	inline void EventBus::unsubscribeAll()
	{
		for (auto it = subscribers.begin(); it != subscribers.end(); it++)
		{
			HandlerList* subs = (*it).second;
			if (subs)
			{
				for (size_t i = 0; i < subs->size(); i++)
				{
					delete (*subs)[i];
				}
				delete subs;
			}
		}
		subscribers.clear();
	}

	inline EventBus& EventBus::get()
	{
		static EventBus instance;
		return instance;
	}
}