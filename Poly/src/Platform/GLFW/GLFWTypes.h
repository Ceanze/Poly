#pragma once

#include "Poly/Core/Input/Keys.h"
#include <GLFW/glfw3.h>

namespace Poly
{
	inline EKey ConvertToPolyKey(int GLFWKey)
	{
		switch (GLFWKey)
		{
		// -------------- Keyboard -------------
		// F1-F12
		case GLFW_KEY_F1:	return EKey::F1;
		case GLFW_KEY_F2:	return EKey::F2;
		case GLFW_KEY_F3:	return EKey::F3;
		case GLFW_KEY_F4:	return EKey::F4;
		case GLFW_KEY_F5:	return EKey::F5;
		case GLFW_KEY_F6:	return EKey::F6;
		case GLFW_KEY_F7:	return EKey::F7;
		case GLFW_KEY_F8:	return EKey::F8;
		case GLFW_KEY_F9:	return EKey::F9;
		case GLFW_KEY_F10:	return EKey::F10;
		case GLFW_KEY_F11:	return EKey::F11;
		case GLFW_KEY_F12:	return EKey::F12;

		// Keypad
		case GLFW_KEY_KP_0:			return EKey::KEYPAD_0;
		case GLFW_KEY_KP_1:			return EKey::KEYPAD_1;
		case GLFW_KEY_KP_2:			return EKey::KEYPAD_2;
		case GLFW_KEY_KP_3:			return EKey::KEYPAD_3;
		case GLFW_KEY_KP_4:			return EKey::KEYPAD_4;
		case GLFW_KEY_KP_5:			return EKey::KEYPAD_5;
		case GLFW_KEY_KP_6:			return EKey::KEYPAD_6;
		case GLFW_KEY_KP_7:			return EKey::KEYPAD_7;
		case GLFW_KEY_KP_8:			return EKey::KEYPAD_8;
		case GLFW_KEY_KP_9:			return EKey::KEYPAD_9;
		case GLFW_KEY_KP_DECIMAL:	return EKey::KEYPAD_DECIMAL;
		case GLFW_KEY_KP_DIVIDE:	return EKey::KEYPAD_DIVIDE;
		case GLFW_KEY_KP_MULTIPLY:	return EKey::KEYPAD_MULTIPLY;
		case GLFW_KEY_KP_SUBTRACT:	return EKey::KEYPAD_SUBTRACT;
		case GLFW_KEY_KP_ADD:		return EKey::KEYPAD_ADD	;
		case GLFW_KEY_KP_ENTER:		return EKey::KEYPAD_ENTER;
		case GLFW_KEY_KP_EQUAL:		return EKey::KEYPAD_EQUAL;

		// Letters
		case GLFW_KEY_A:	return EKey::A;
		case GLFW_KEY_B:	return EKey::B;
		case GLFW_KEY_C:	return EKey::C;
		case GLFW_KEY_D:	return EKey::D;
		case GLFW_KEY_E:	return EKey::E;
		case GLFW_KEY_F:	return EKey::F;
		case GLFW_KEY_G:	return EKey::G;
		case GLFW_KEY_H:	return EKey::H;
		case GLFW_KEY_I:	return EKey::I;
		case GLFW_KEY_J:	return EKey::J;
		case GLFW_KEY_K:	return EKey::K;
		case GLFW_KEY_L:	return EKey::L;
		case GLFW_KEY_M:	return EKey::M;
		case GLFW_KEY_N:	return EKey::N;
		case GLFW_KEY_O:	return EKey::O;
		case GLFW_KEY_P:	return EKey::P;
		case GLFW_KEY_Q:	return EKey::Q;
		case GLFW_KEY_R:	return EKey::R;
		case GLFW_KEY_S:	return EKey::S;
		case GLFW_KEY_T:	return EKey::T;
		case GLFW_KEY_U:	return EKey::U;
		case GLFW_KEY_V:	return EKey::V;
		case GLFW_KEY_W:	return EKey::W;
		case GLFW_KEY_X:	return EKey::X;
		case GLFW_KEY_Y:	return EKey::Y;
		case GLFW_KEY_Z:	return EKey::Z;

		// Special
		case GLFW_KEY_ESCAPE:			return EKey::ESC;
		case GLFW_KEY_TAB:				return EKey::TAB;
		case GLFW_KEY_LEFT_SHIFT:		return EKey::LSHIFT;
		case GLFW_KEY_RIGHT_SHIFT:		return EKey::RSHIFT;
		case GLFW_KEY_LEFT_CONTROL:		return EKey::LCTRL;
		case GLFW_KEY_RIGHT_CONTROL:	return EKey::RCTRL;
		case GLFW_KEY_LEFT_ALT:			return EKey::LALT;
		case GLFW_KEY_RIGHT_ALT:		return EKey::RALT;
		case GLFW_KEY_SPACE:			return EKey::SPACE;
		case GLFW_KEY_CAPS_LOCK:		return EKey::CAPS_LOCK;
		case GLFW_KEY_SCROLL_LOCK:		return EKey::SCROLL_LOCK;
		case GLFW_KEY_NUM_LOCK:			return EKey::NUM_LOCK;
		case GLFW_KEY_BACKSPACE:		return EKey::BACKSPACE;
		case GLFW_KEY_ENTER:			return EKey::ENTER;
		case GLFW_KEY_DELETE:			return EKey::DEL;
		case GLFW_KEY_LEFT:				return EKey::LEFT;
		case GLFW_KEY_RIGHT:			return EKey::RIGHT;
		case GLFW_KEY_UP:				return EKey::UP;
		case GLFW_KEY_DOWN:				return EKey::DOWN;
		case GLFW_KEY_PAGE_UP:			return EKey::PAGE_UP;
		case GLFW_KEY_PAGE_DOWN:		return EKey::PAGE_DOWN;
		case GLFW_KEY_HOME:				return EKey::HOME;
		case GLFW_KEY_END:				return EKey::END;
		case GLFW_KEY_INSERT:			return EKey::INSERT;
		case GLFW_KEY_PRINT_SCREEN:		return EKey::PRINT_SCREEN;
		case GLFW_KEY_PAUSE:			return EKey::PAUSE;
		case GLFW_KEY_MENU:				return EKey::MENU;
		case GLFW_KEY_APOSTROPHE:		return EKey::APOSTROPHE;
		case GLFW_KEY_COMMA:			return EKey::COMMA;
		case GLFW_KEY_PERIOD:			return EKey::PERIOD;
		case GLFW_KEY_MINUS:			return EKey::MINUS;
		case GLFW_KEY_SLASH:			return EKey::SLASH;
		case GLFW_KEY_SEMICOLON:		return EKey::SEMICOLON;
		case GLFW_KEY_EQUAL:			return EKey::EQUAL;
		case GLFW_KEY_LEFT_BRACKET:		return EKey::LBRACKET;
		case GLFW_KEY_RIGHT_BRACKET:	return EKey::RBRACKET;
		case GLFW_KEY_BACKSLASH:		return EKey::BACKSLASH;

		// Mouse
		case GLFW_MOUSE_BUTTON_1:		return EKey::MOUSE_1; // EKey::LEFT
		case GLFW_MOUSE_BUTTON_2:		return EKey::MOUSE_2; // EKey::RIGHT
		case GLFW_MOUSE_BUTTON_3:		return EKey::MOUSE_3; // EKey::MIDDLE
		case GLFW_MOUSE_BUTTON_4:		return EKey::MOUSE_4;
		case GLFW_MOUSE_BUTTON_5:		return EKey::MOUSE_5;
		case GLFW_MOUSE_BUTTON_6:		return EKey::MOUSE_6;
		case GLFW_MOUSE_BUTTON_7:		return EKey::MOUSE_7;
		case GLFW_MOUSE_BUTTON_8:		return EKey::MOUSE_8;

		default:						return EKey::UNKNOWN;


		/**
		 * Gamepad support is not yet implemented due to the fact that
		 * glfw does not support gamepad callbacks, only polling.
		 * The current input system only supports callbacks and will need
		 * to be updated to allow for polling as well.
		 * Gamepad support is on hold until that is fixed
		 */
		// Gamepad
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_LEFT_DPAD;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_RIGHT_DPAD;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_UP_DPAD;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_DOWN_DPAD;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_A;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_B;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_X;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_Y;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_CROSS;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_CIRCLE;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_SQUARE;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BUTTON_TRIANGLE;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_BACK;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_GUIDE;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_START;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_LEFT_STICK;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_RIGHT_STICK;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_LEFT_BUMPER;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_RIGHT_BUMPER;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_LEFT_TRIGGER
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_RIGHT_TRIGGER
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_L1;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_L2;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_R1;
		// case GLFW_JOYSTICK_11:	return EKey::GAMEPAD_R2;
		}
	}

	inline FKeyModifier ConvertToPolyModifier(int GLFWModifer)
	{
		FKeyModifier mask = FKeyModifier::NONE;
		if ((GLFWModifer & GLFW_MOD_SHIFT) > 0)		mask |= FKeyModifier::SHIFT;
		if ((GLFWModifer & GLFW_MOD_CONTROL) > 0)	mask |= FKeyModifier::CTRL;
		if ((GLFWModifer & GLFW_MOD_ALT) > 0)		mask |= FKeyModifier::ALT;
		if ((GLFWModifer & GLFW_MOD_CAPS_LOCK) > 0)	mask |= FKeyModifier::CAPS_LOCK;
		if ((GLFWModifer & GLFW_MOD_NUM_LOCK) > 0)	mask |= FKeyModifier::NUM_LOCK;
		return mask;
	}
}