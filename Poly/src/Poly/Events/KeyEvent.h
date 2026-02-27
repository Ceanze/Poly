#pragma once

#include "Poly/Events/Event.h"
#include "Poly/Core/Input/Keys.h"

namespace Poly::Events
{
	class KeyPressed : public Event
	{
	public:
		KeyPressed(EKey key, FKeyModifier keyMod, bool isRepeat) : m_Key(key), m_KeyMod(keyMod), m_IsRepeat(isRepeat) {}

		EKey GetKey() const { return m_Key; }
		FKeyModifier GetKeyModifier() const { return m_KeyMod; }
		bool IsRepeat() const { return m_IsRepeat; }

		DEFINE_EVENT(KeyPressed, EventType::KeyPressed)
		DEFINE_EVENT_CATEGORY(KeyPressed, EventCategory::Key)

	private:
		const EKey m_Key;
		const FKeyModifier m_KeyMod;
		const bool m_IsRepeat;
	};

	class KeyReleased: public Event
	{
	public:
		KeyReleased(EKey key, FKeyModifier keyMod) : m_Key(key), m_KeyMod(keyMod) {}

		EKey GetKey() const { return m_Key; }
		FKeyModifier GetKeyModifier() const { return m_KeyMod; }

		DEFINE_EVENT(KeyReleased, EventType::KeyReleased)
		DEFINE_EVENT_CATEGORY(KeyReleased, EventCategory::Key)

	private:
		const EKey m_Key;
		const FKeyModifier m_KeyMod;
	};

	class KeyTyped : public Event
	{
	public:
		KeyTyped(EKey key) : m_Key(key) {}

		EKey GetKey() const { return m_Key; }

		DEFINE_EVENT(KeyTyped, EventType::KeyTyped)
		DEFINE_EVENT_CATEGORY(KeyTyped, EventCategory::Key)

	private:
		const EKey m_Key;
	};
}