#pragma once

#include "Poly/Events/Event.h"

namespace Poly::Events
{
	class WindowClosed : public Event
	{
	public:
		DEFINE_EVENT(WindowClosed, EventType::WindowClosed)
		DEFINE_EVENT_CATEGORY(WindowClosed, EventCategory::Window)
	};

	class WindowResized : public Event
	{
	public:
		WindowResized(unsigned width, unsigned height) : m_Width(width), m_Height(height) {}

		unsigned GetWidth() const { return m_Width; }
		unsigned GetHeight() const { return m_Height; }

		DEFINE_EVENT(WindowResized, EventType::WindowResized)
		DEFINE_EVENT_CATEGORY(WindowResized, EventCategory::Window)

	private:
		const unsigned m_Width;
		const unsigned m_Height;
	};

	class WindowMoved : public Event
	{
	public:
		WindowMoved(int x, int y) : m_X(x), m_Y(y) {}

		int GetX() const { return m_X; }
		int GetY() const { return m_Y; }

		DEFINE_EVENT(WindowMoved, EventType::WindowMoved)
		DEFINE_EVENT_CATEGORY(WindowMoved, EventCategory::Window)

	private:
		const int m_X;
		const int m_Y;
	};
}