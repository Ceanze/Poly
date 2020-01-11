#pragma once

#include "Event.h"

/**
	Classes in order to store member function pointers.
**/

namespace Poly
{
	template<class T, class EventType>
	unsigned getID(T* instance)
	{
		size_t h = typeid(EventType).hash_code();
		return (unsigned)(h - (size_t)instance);
	}

	class HandlerFunctionBase
	{
	public:
		bool exec(Event* e) { return call(e); }
		unsigned ID = 0;

	private:
		virtual bool call(Event* e) = 0;
	};

	template<class T, class EventType>
	class MemberFunctionHandler : public HandlerFunctionBase
	{
	public:
		typedef bool (T::* MemberFunction)(EventType*);

		MemberFunctionHandler(T* instance, MemberFunction memberFunction)
			: instance(instance), memberFunction(memberFunction) {
			ID = getID<T, EventType>(instance);
		};

		bool call(Event* e) { return (instance->*memberFunction)(static_cast<EventType*>(e)); }

	private:
		T* instance;
		MemberFunction memberFunction;
	};

}