#pragma once

namespace Poly
{
	enum class FKeyModifier : uint16
	{
		NONE		= FLAG(0),
		SHIFT		= FLAG(1),
		CTRL		= FLAG(2),
		ALT			= FLAG(3),
		CAPS_LOCK	= FLAG(4),
		NUM_LOCK	= FLAG(5),
	};
	ENABLE_BITMASK_OPERATORS(FKeyModifier);

	enum class EKey : uint16
	{
		UNKNOWN		= 0,

		// -------------- Keyboard -------------
		// F1-F12
		F1	= 1,
		F2	= 2,
		F3	= 3,
		F4	= 4,
		F5	= 5,
		F6	= 6,
		F7	= 7,
		F8	= 8,
		F9	= 9,
		F10	= 10,
		F11	= 11,
		F12	= 12,

		// Keypad
		KEYPAD_0		= 20,
		KEYPAD_1		= 21,
		KEYPAD_2		= 22,
		KEYPAD_3		= 23,
		KEYPAD_4		= 24,
		KEYPAD_5		= 25,
		KEYPAD_6		= 26,
		KEYPAD_7		= 27,
		KEYPAD_8		= 28,
		KEYPAD_9		= 29,
		KEYPAD_DECIMAL	= 30,
		KEYPAD_DIVIDE	= 31,
		KEYPAD_MULTIPLY	= 32,
		KEYPAD_SUBTRACT	= 33,
		KEYPAD_ADD		= 34,
		KEYPAD_ENTER	= 35,
		KEYPAD_EQUAL	= 36,

		// Letters
		A	= 40,
		B	= 41,
		C	= 42,
		D	= 43,
		E	= 44,
		F	= 45,
		G	= 46,
		H	= 47,
		I	= 48,
		J	= 49,
		K	= 50,
		L	= 51,
		M	= 52,
		N	= 53,
		O	= 54,
		P	= 55,
		Q	= 56,
		R	= 57,
		S	= 58,
		T	= 59,
		U	= 60,
		V	= 60,
		W	= 62,
		X	= 63,
		Y	= 64,
		Z	= 65,

		// Special
		ESC				= 70,
		TAB				= 71,
		LSHIFT			= 72,
		RSHIFT			= 73,
		LCTRL			= 74,
		RCTRL			= 75,
		LALT			= 76,
		RALT			= 77,
		SPACE			= 78,
		CAPS_LOCK		= 79, // nice
		SCROLL_LOCK		= 80,
		NUM_LOCK		= 81,
		BACKSPACE		= 82,
		ENTER			= 83,
		DEL				= 84,
		LEFT			= 85,
		RIGHT			= 86,
		UP				= 87,
		DOWN			= 88,
		PAGE_UP			= 89,
		PAGE_DOWN		= 90,
		HOME			= 91,
		END				= 92,
		INSERT			= 93,
		PRINT_SCREEN	= 94,
		PAUSE			= 95,
		MENU			= 96, // Windows button
		APOSTROPHE		= 97, // '
		COMMA			= 98, // ,
		PERIOD			= 99, // .
		MINUS			= 100, // -
		SLASH			= 101, // /
		SEMICOLON		= 102, // ;
		EQUAL			= 103, // =
		LBRACKET		= 104, // [
		RBRACKET		= 105, // ]
		BACKSLASH		= 106, // \

		// -------------- Mouse -------------
		MOUSE_1			= 110,
		MOUSE_2			= 111,
		MOUSE_3			= 112,
		MOUSE_4			= 113,
		MOUSE_5			= 114,
		MOUSE_6			= 115,
		MOUSE_7			= 116,
		MOUSE_8			= 117,
		MOUSE_LEFT		= MOUSE_1,
		MOUSE_RIGHT		= MOUSE_2,
		MOUSE_MIDDLE	= MOUSE_3,

		// -------------- Gamepad -------------
		GAMEPAD_LEFT_DPAD		= 120,
		GAMEPAD_RIGHT_DPAD		= 121,
		GAMEPAD_UP_DPAD			= 122,
		GAMEPAD_DOWN_DPAD		= 123,
		GAMEPAD_BUTTON_A		= 124, // Xbox naming
		GAMEPAD_BUTTON_B		= 125, // Xbox naming
		GAMEPAD_BUTTON_X		= 126, // Xbox naming
		GAMEPAD_BUTTON_Y		= 127, // Xbox naming
		GAMEPAD_BUTTON_CROSS	= GAMEPAD_BUTTON_A,	// PlayStation naming
		GAMEPAD_BUTTON_CIRCLE	= GAMEPAD_BUTTON_B,	// PlayStation naming
		GAMEPAD_BUTTON_SQUARE	= GAMEPAD_BUTTON_X,	// PlayStation naming
		GAMEPAD_BUTTON_TRIANGLE	= GAMEPAD_BUTTON_Y,	// PlayStation naming
		GAMEPAD_BACK			= 128,
		GAMEPAD_GUIDE			= 129,
		GAMEPAD_START			= 130,
		GAMEPAD_LEFT_STICK		= 131, // Xbox naming
		GAMEPAD_RIGHT_STICK		= 132, // Xbox naming
		GAMEPAD_LEFT_BUMPER		= 133, // Xbox naming
		GAMEPAD_RIGHT_BUMPER	= 134, // Xbox naming
		GAMEPAD_LEFT_TRIGGER	= 135,
		GAMEPAD_RIGHT_TRIGGER	= 136,
		GAMEPAD_L1				= GAMEPAD_LEFT_BUMPER,		// PlayStation naming
		GAMEPAD_L2				= GAMEPAD_LEFT_TRIGGER,		// PlayStation naming
		GAMEPAD_R1				= GAMEPAD_RIGHT_BUMPER,		// PlayStation naming
		GAMEPAD_R2				= GAMEPAD_RIGHT_TRIGGER,	// PlayStation naming
	};
}