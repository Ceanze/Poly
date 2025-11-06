#include "polypch.h"
#include "InputManager.h"

namespace Poly
{
	InputManager::MouseProperties InputManager::s_MouseProperties;
	InputManager::KeyProperties InputManager::s_KeyProperties;

	void InputManager::Update()
	{
		s_KeyProperties.PreviousKeys = s_KeyProperties.CurrentKeys;
		s_MouseProperties.DeltaX = 0.0;
		s_MouseProperties.DeltaY = 0.0;
		s_MouseProperties.ScrollDeltaY = 0.0;
		s_MouseProperties.ScrollDeltaY = 0.0;
	}

	bool InputManager::IsKeyDown(EKey key)
	{
		return s_KeyProperties.CurrentKeys[ENUM_CAST(EKey, key)];
	}

	bool InputManager::IsKeyPressed(EKey key)
	{
		return IsKeyDown(key) && !s_KeyProperties.PreviousKeys[ENUM_CAST(EKey, key)];
	}

	bool InputManager::IsKeyReleased(EKey key)
	{
		return !IsKeyDown(key) && s_KeyProperties.PreviousKeys[ENUM_CAST(EKey, key)];
	}

	bool InputManager::IsModifier(FKeyModifier mod)
	{
		return BitsSet(s_KeyProperties.CurrentMods, mod);
	}

	double InputManager::GetMouseX()
	{
		return s_MouseProperties.X;
	}

	double InputManager::GetMouseY()
	{
		return s_MouseProperties.Y;
	}

	double InputManager::GetMouseDeltaX()
	{
		return s_MouseProperties.DeltaX;
	}

	double InputManager::GetMouseDeltaY()
	{
		return s_MouseProperties.DeltaY;
	}

	double InputManager::GetScrollDeltaX()
	{
		return s_MouseProperties.ScrollDeltaX;
	}

	double InputManager::GetScrollDeltaY()
	{
		return s_MouseProperties.ScrollDeltaY;
	}

	void InputManager::KeyCallback(EKey key, FKeyModifier mods, EKeyAction action)
	{
		s_KeyProperties.CurrentKeys[ENUM_CAST(EKey, key)] = action == EKeyAction::PRESS;
		s_KeyProperties.CurrentMods = mods;
	}

	void InputManager::MouseCallback(double x, double y)
	{
		s_MouseProperties.DeltaX = x - s_MouseProperties.X;
		s_MouseProperties.DeltaY = y - s_MouseProperties.Y;

		s_MouseProperties.X = x;
		s_MouseProperties.Y = y;
	}

	void InputManager::ScrollCallback(double x, double y)
	{
		s_MouseProperties.ScrollDeltaX = x - s_MouseProperties.ScrollDeltaX;
		s_MouseProperties.ScrollDeltaY = y - s_MouseProperties.ScrollDeltaY;
	}
}
