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
		static void setKeyPressed(int key);
		static void setKeyReleased(int key);
		static void setMouseDelta(double x, double y);

		static bool isKeyPressed(int key);
		static bool isKeyReleased(int key);
		static bool isKeyToggled(int key);
		static bool isKey(int key, KeyState keyState);
		static KeyState getKeyState(int key);
		static glm::vec2 getMouseDelta();

		// Sets all keys to released, removes toggles and resets mouse delta
		static void reset();

	private:
		static glm::vec2 mouseDelta;
		static std::unordered_map<int, KeyState> keys;
	};

}