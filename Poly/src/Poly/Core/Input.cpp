#include "polypch.h"
#include "Input.h"

#include "PolyUtils.h"

namespace Poly
{
	glm::vec2 Input::mouseDelta = { 0.f, 0.f };
	std::unordered_map<int, Input::KeyState> Input::keys;
	ENABLE_BITMASK_OPERATORS(Input::KeyState);

	void Input::setKeyPressed(int key)
	{
		auto& keyIt = keys.find(key);
		if (keyIt == keys.end())
			keys[key] = KeyState::PRESSED | KeyState::TOGGLED;
		else {
			keys[key] |= KeyState::PRESSED;
			keys[key] &= ~KeyState::RELEASED;
			keys[key] ^= KeyState::TOGGLED;
		}
	}

	void Input::setKeyReleased(int key)
	{
		auto& keyIt = keys.find(key);
		if (keyIt == keys.end())
			keys[key] = KeyState::RELEASED;
		else {
			keys[key] |= KeyState::RELEASED;
			keys[key] &= ~KeyState::PRESSED;
		}
	}

	void Input::setMouseDelta(double x, double y)
	{
		mouseDelta = { x, y };
	}

	bool Input::isKeyPressed(int key)
	{
		return isKey(key, KeyState::PRESSED);
	}

	bool Input::isKeyReleased(int key)
	{
		return isKey(key, KeyState::RELEASED);
	}

	bool Input::isKeyToggled(int key)
	{
		return isKey(key, KeyState::TOGGLED);
	}

	bool Input::isKey(int key, KeyState keyState)
	{
		auto& keyIt = keys.find(key);
		if (keyIt == keys.end())
			return false;
		else
			return (keys[key] & keyState) == keyState;
	}

	Input::KeyState Input::getKeyState(int key)
	{
		auto& keyIt = keys.find(key);
		if (keyIt == keys.end())
			return KeyState::NONE;
		else
			return keys[key];
	}

	glm::vec2 Input::getMouseDelta()
	{
		return mouseDelta;
	}

	void Input::reset()
	{
		mouseDelta = { 0.f, 0.f };
		for (auto& key : keys)
			key.second = KeyState::NONE;
	}

}