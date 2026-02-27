#pragma once

#include "Poly/Events/Event.h"
#include "Poly/Core/Input/Keys.h"

namespace Poly::Events
{
	class MouseButtonPressed : public Event
	{
	public:
		MouseButtonPressed(EKey button, FKeyModifier buttonMod) : m_Button(button), m_ButtonMod(buttonMod) {}

		EKey GetButton() const { return m_Button; }
		FKeyModifier GetButtonModifier() const { return m_ButtonMod; }

		DEFINE_EVENT(MouseButtonPressed, EventType::MouseButtonPressed)

	private:
		const EKey m_Button;
		const FKeyModifier m_ButtonMod;
	};

	class MouseButtonReleased : public Event
	{
	public:
		MouseButtonReleased(EKey button, FKeyModifier buttonMod) : m_Button(button), m_ButtonMod(buttonMod) {}

		EKey GetButton() const { return m_Button; }
		FKeyModifier GetButtonModifier() const { return m_ButtonMod; }

		DEFINE_EVENT(MouseButtonReleased, EventType::MouseButtonReleased)

	private:
		const EKey m_Button;
		const FKeyModifier m_ButtonMod;
	};

	class MouseMoved : public Event
	{
	public:
		MouseMoved(double x, double y, double deltaX, double deltaY) : m_X(x), m_Y(y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}

		double GetX() const { return m_X; }
		double GetY() const { return m_Y; }
		double GetDeltaX() const { return m_DeltaX; }
		double GetDeltaY() const { return m_DeltaY; }

		DEFINE_EVENT(MouseMoved, EventType::MouseMoved)

	private:
		const double m_X;
		const double m_Y;
		const double m_DeltaX;
		const double m_DeltaY;
	};

	class MouseScrolled : public Event
	{
	public:
		MouseScrolled(double x, double y, double deltaX, double deltaY) : m_X(x), m_Y(y), m_DeltaX(deltaX), m_DeltaY(deltaY) {}

		double GetX() const { return m_X; }
		double GetY() const { return m_Y; }
		double GetDeltaX() const { return m_DeltaX; }
		double GetDeltaY() const { return m_DeltaY; }

		DEFINE_EVENT(MouseScrolled, EventType::MouseScrolled)

	private:
		const double m_X;
		const double m_Y;
		const double m_DeltaX;
		const double m_DeltaY;
	};
}