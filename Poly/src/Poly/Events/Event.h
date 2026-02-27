#pragma once

namespace Poly
{
	enum class EventType
	{
		None = 0,
		WindowClosed,
		WindowResized,
		WindowMoved,

		KeyPressed,
		KeyReleased,
		KeyTyped,

		MouseButtonPressed,
		MouseButtonReleased,
		MouseMoved,
		MouseScrolled
	};

#define DEFINE_EVENT(type, eventType)\
	static EventType GetStaticType() { return eventType; }\
	virtual EventType GetEventType() const override { return eventType; }\
	virtual const char* GetName() const override { return #type; }

	class Event
	{
	public:
		virtual ~Event() = default;

		bool Handled = false;

		virtual EventType GetEventType() const = 0;
		virtual const char* GetName() const = 0;
	};

	class EventDispatcher
	{
	public:
		EventDispatcher(Event& event) : m_Event(event) {}

		template <typename Type, typename Func>
		bool Dispatch(const Func& func)
		{
			if (Type::GetStaticType() == m_Event.GetEventType())
			{
				m_Event.Handled |= func(static_cast<Type&>(m_Event));
				return true;
			}

			return false;
		}

	private:
		Event& m_Event;
	};
}