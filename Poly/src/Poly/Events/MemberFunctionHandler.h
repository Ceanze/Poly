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
		return static_cast<unsigned>(h - reinterpret_cast<size_t>(instance));
	}

	class HandlerFunctionBase
	{
	public:
		void exec(Event* e) { call(e); }
		unsigned ID = 0;

	private:
		virtual void call(Event* e) = 0;
	};

	template<class T, class EventType>
	class MemberFunctionHandler : public HandlerFunctionBase
	{
	public:
		typedef void (T::* MemberFunction)(EventType*);

		MemberFunctionHandler(T* instance, MemberFunction memberFunction)
			: instance(instance), memberFunction(memberFunction) {
			ID = getID<T, EventType>(instance);
		};

		void call(Event* e) { return (instance->*memberFunction)(static_cast<EventType*>(e)); }

	private:
		T* instance;
		MemberFunction memberFunction;
	};

}