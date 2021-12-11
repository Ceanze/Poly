#pragma once

/*
	Input is a static class that stores all the pressed down keys and toggled keys, as well as the mouse delta for current frame
*/

#include "polypch.h"

namespace Poly
{

	class Input
	{
	public:
		enum class KeyState
		{
			NONE = 0,
			RELEASED = 1,
			PRESSED = 2,
			REPEAT = 4,
			TOGGLED = 8/*,
			FIRST_PRESSED = 16,
			FIRST_RELEASED = 32,*/
		};
	public:
		static void SetKeyPressed(int key);
		static void SetKeyReleased(int key);
		static void SetMouseDelta(double x, double y);
		static void SetMousePosition(double x, double y);

		static bool IsKeyPressed(int key);
		static bool IsKeyReleased(int key);
		static bool IsKeyToggled(int key);
		static bool IsKey(int key, KeyState keyState);
		static KeyState GetKeyState(int key);
		static glm::vec2 GetMouseDelta();
		static glm::vec2 GetMousePosition();

		// Sets all keys to released, removes toggles and resets mouse delta
		static void Reset();

	private:
		static glm::dvec2 s_MouseDelta;
		static glm::dvec2 s_MousePos;
		static std::unordered_map<int, KeyState> s_Keys;
	};

	ENABLE_BITMASK_OPERATORS(Input::KeyState);
}