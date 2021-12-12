#pragma once
#include "KeyCode.h"
#include "Keys.h"

/*
	Input is a static class that stores all the pressed down keys and toggled keys, as well as the mouse delta for current frame
*/

namespace Poly
{
	class Input
	{
	public:
		enum class FKeyState
		{
			NONE		= FLAG(0),
			RELEASED	= FLAG(1),
			PRESSED		= FLAG(2),
			REPEAT		= FLAG(4),
			TOGGLED		= FLAG(8),
			// FIRST_PRESSED = 16,
			// FIRST_RELEASED = 32,
		};
	public:
		static void SetKeyPressed(KeyCode key);
		static void SetKeyReleased(KeyCode key);
		static void SetMouseDelta(double x, double y);
		static void SetMousePosition(double x, double y);

		static bool IsKeyPressed(KeyCode key);
		static bool IsKeyReleased(KeyCode key);
		static bool IsKeyToggled(KeyCode key);
		static bool IsKey(KeyCode key, FKeyState keyState);
		static FKeyState GetKeyState(KeyCode key);
		static glm::vec2 GetMouseDelta();
		static glm::vec2 GetMousePosition();

		// Sets all keys to released, removes toggles and resets mouse delta
		static void Reset();

	private:
		static glm::dvec2 s_MouseDelta;
		static glm::dvec2 s_MousePos;
		static std::unordered_map<KeyCode, FKeyState> s_Keys;
	};

	ENABLE_BITMASK_OPERATORS(Input::FKeyState);
}