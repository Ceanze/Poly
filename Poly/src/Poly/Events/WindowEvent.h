#pragma once

#include "Poly/Events/Event.h"

namespace Poly
{
	class WindowCloseEvent : public Event
	{
	public:
		DEFINE_EVENT(WindowCloseEvent, EventType::WINDOW_CLOSE)
	};

	class WindowResizeEvent : public Event
	{
	public:
		WindowResizeEvent(unsigned width, unsigned height) : m_Width(width), m_Height(height) {}

		unsigned GetWidth() const { return m_Width; }
		unsigned GetHeight() const { return m_Height; }

		DEFINE_EVENT(WindowResizeEvent, EventType::WINDOW_RESIZE)

	private:
		const unsigned m_Width;
		const unsigned m_Height;
	};
}