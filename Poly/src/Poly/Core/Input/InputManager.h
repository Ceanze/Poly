#pragma once

#include "Keys.h"

#include <bitset>

namespace Poly
{
	class InputManager
	{
	public:
		static void Update();

		static bool IsKeyDown(EKey key);
		static bool IsKeyPressed(EKey key);
		static bool IsKeyReleased(EKey key);
		static bool IsModifier(FKeyModifier mod);

		static double GetMouseX();
		static double GetMouseY();
		static double GetMouseDeltaX();
		static double GetMouseDeltaY();
		static double GetScrollDeltaX();
		static double GetScrollDeltaY();

		static void KeyCallback(EKey key, FKeyModifier mods, EKeyAction action);
		static void MouseCallback(double x, double y);
		static void ScrollCallback(double x, double y);

	private:
		struct MouseProperties
		{
			double X;
			double Y;
			double DeltaX;
			double DeltaY;
			double ScrollDeltaX;
			double ScrollDeltaY;
		};

		struct KeyProperties
		{
			std::bitset<256> CurrentKeys;
			std::bitset<256> PreviousKeys;
			FKeyModifier CurrentMods;
		};

		static MouseProperties s_MouseProperties;
		static KeyProperties s_KeyProperties;
	};
}

// Action system
//{
//	// Create/Get action
//	InputAction action = InputContext.AddAction("move", EActionType::Axis2D);
//	InputAction action = InputContext.GetAction("move");
//
//	// Add bindings (only Axis2D is valid because of AddAction, rest are examples)
//	action.BindButton(Key::W);
//	action.BindAxis(Key::W, Key::S);
//	action.BindAxis2D({ Key::W, Key::S }, { Key::A, Key::D });
//
//	// Callbacks
//	action.ButtonCallback([](EActionPhase) {}); // Started, Performing, Stopped (or similar)
//	action.Axis1dCallback([](float x) {}); // [-1.0, 1.0]
//	action.Axis2dCallback([](float x, float y) {}); // [-1.0, 1.0], [-1.0, 1.0]
//
//	// Polling
//	action.IsStarted();
//	action.IsPerforming();
//	action.IsStopped();
//	action.GetAxis();
//	action.GetAxis2D();
//}