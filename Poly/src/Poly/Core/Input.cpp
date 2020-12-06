#include "polypch.h"
#include "Input.h"

#include "PolyUtils.h"

namespace Poly
{
	glm::vec2 Input::s_MouseDelta = { 0.f, 0.f };
	std::unordered_map<int, Input::KeyState> Input::s_Keys;
	ENABLE_BITMASK_OPERATORS(Input::KeyState);

	void Input::SetKeyPressed(int key)
	{
		auto keyIt = s_Keys.find(key);
		if (keyIt == s_Keys.end())
			s_Keys[key] = KeyState::PRESSED | KeyState::TOGGLED;
		else {
			s_Keys[key] |= KeyState::PRESSED;
			s_Keys[key] &= ~KeyState::RELEASED;
			s_Keys[key] ^= KeyState::TOGGLED;
		}
	}

	void Input::SetKeyReleased(int key)
	{
		auto keyIt = s_Keys.find(key);
		if (keyIt == s_Keys.end())
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
		auto keyIt = s_Keys.find(key);
		if (keyIt == s_Keys.end())
			return false;
		else
			return (s_Keys[key] & keyState) == keyState;
	}

	Input::KeyState Input::GetKeyState(int key)
	{
		auto keyIt = s_Keys.find(key);
		if (keyIt == s_Keys.end())
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

	void Input::Reset()
	{
		s_MouseDelta = { 0.f, 0.f };
		for (auto& key : s_Keys)
			key.second = KeyState::NONE;
	}

}