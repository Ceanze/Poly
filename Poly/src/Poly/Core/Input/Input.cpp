#include "polypch.h"
#include "Input.h"

#include "Poly/Core/PolyUtils.h"

namespace Poly
{
	glm::dvec2 Input::s_MouseDelta = { 0.f, 0.f };
	glm::dvec2 Input::s_MousePos = { 0.f, 0.f };
	std::unordered_map<int, Input::KeyState> Input::s_Keys;

	void Input::SetKeyPressed(int key)
	{
		if (!s_Keys.contains(key))
			s_Keys[key] = KeyState::PRESSED | KeyState::TOGGLED;
		else {
			s_Keys[key] |= KeyState::PRESSED;
			s_Keys[key] &= ~KeyState::RELEASED;
			s_Keys[key] ^= KeyState::TOGGLED;
		}
	}

	void Input::SetKeyReleased(int key)
	{
		if (!s_Keys.contains(key))
			s_Keys[key] = KeyState::RELEASED;
		else {
			s_Keys[key] |= KeyState::RELEASED;
			s_Keys[key] &= ~KeyState::PRESSED;
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

	bool Input::IsKeyPressed(int key)
	{
		return IsKey(key, KeyState::PRESSED);
	}

	bool Input::IsKeyReleased(int key)
	{
		return IsKey(key, KeyState::RELEASED);
	}

	bool Input::IsKeyToggled(int key)
	{
		return IsKey(key, KeyState::TOGGLED);
	}

	bool Input::IsKey(int key, KeyState keyState)
	{
		if (!s_Keys.contains(key))
			return false;
		else
			return (s_Keys[key] & keyState) == keyState;
	}

	Input::KeyState Input::GetKeyState(int key)
	{
		if (!s_Keys.contains(key))
			return KeyState::NONE;
		else
			return s_Keys[key];
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
			key.second = KeyState::NONE;
	}

}