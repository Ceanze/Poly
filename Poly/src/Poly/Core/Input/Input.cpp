#include "polypch.h"
#include "Input.h"

#include "Poly/Core/PolyUtils.h"

namespace Poly
{
	glm::dvec2 Input::s_MouseDelta = { 0.f, 0.f };
	glm::dvec2 Input::s_MousePos = { 0.f, 0.f };
	std::unordered_map<KeyCode, Input::FKeyState> Input::s_Keys;

	void Input::SetKeyPressed(KeyCode keyCode)
	{
		if (!s_Keys.contains(keyCode))
			s_Keys[keyCode] = FKeyState::PRESSED | FKeyState::TOGGLED;
		else {
			s_Keys[keyCode] |= FKeyState::PRESSED;
			s_Keys[keyCode] &= ~FKeyState::RELEASED;
			s_Keys[keyCode] ^= FKeyState::TOGGLED;
		}
	}

	void Input::SetKeyReleased(KeyCode keyCode)
	{
		if (!s_Keys.contains(keyCode))
			s_Keys[keyCode] = FKeyState::RELEASED;
		else {
			s_Keys[keyCode] |= FKeyState::RELEASED;
			s_Keys[keyCode] &= ~FKeyState::PRESSED;
		}
	}

	void Input::SetMouseDelta(double x, double y)
	{
		s_MouseDelta.x = x;
		s_MouseDelta.y = y;
	}

	void Input::SetMousePosition(double x, double y)
	{
		s_MousePos.x = x;
		s_MousePos.y = y;
	}

	bool Input::IsKeyPressed(KeyCode keyCode)
	{
		return IsKey(keyCode, FKeyState::PRESSED);
	}

	bool Input::IsKeyReleased(KeyCode keyCode)
	{
		return IsKey(keyCode, FKeyState::RELEASED);
	}

	bool Input::IsKeyToggled(KeyCode keyCode)
	{
		return IsKey(keyCode, FKeyState::TOGGLED);
	}

	bool Input::IsKey(KeyCode keyCode, FKeyState keyState)
	{
		if (!s_Keys.contains(keyCode))
			return false;
		else
			return (s_Keys[keyCode] & keyState) == keyState;
	}

	Input::FKeyState Input::GetKeyState(KeyCode keyCode)
	{
		if (!s_Keys.contains(keyCode))
			return FKeyState::NONE;
		else
			return s_Keys[keyCode];
	}

	glm::vec2 Input::GetMouseDelta()
	{
		glm::vec2 md(s_MouseDelta);
		s_MouseDelta = { 0.f, 0.f };
		return md;
	}

	glm::vec2 Input::GetMousePosition()
	{
		return s_MousePos;
	}

	void Input::Reset()
	{
		s_MouseDelta = { 0.f, 0.f };
		for (auto& key : s_Keys)
			key.second = FKeyState::NONE;
	}

}