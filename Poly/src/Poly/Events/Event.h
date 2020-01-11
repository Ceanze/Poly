#pragma once

/**
	When defining an event, add the EVENT_CLASS_TYPE to make sure the events will work correctly.
**/

namespace Poly {

	enum class EventType
	{
		None = 0,
		KeyPress, KeyRelease, KeyTyped
	};

	class Event
	{
	public:
		bool handled = false;

		//virtual EventType getEventType() const = 0;
	};

//#define EVENT_CLASS_TYPE(type)	static EventType getStaticEventType() { return EventType::type; }\
//								virtual EventType getEventType() const override { return getStaticEventType(); }

	struct TestEvent : Event
	{
		TestEvent(int test) : test(test) {};
		int test;
	};

}