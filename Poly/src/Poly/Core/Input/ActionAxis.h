#pragma once

#include "Keys.h"

namespace Poly
{
	enum class EMouseMove
	{
		MOVE_LEFT,
		MOVE_RIGHT,
		MOVE_UP,
		MOVE_DOWN
	};

	class ActionAxis
	{
	public:
		ActionAxis() = default;
		ActionAxis(EKey negativeKey, EKey positiveKey) : m_KeyPair(negativeKey, positiveKey), m_IsKeyPair(true) {}
		ActionAxis(EMouseMove negativeMove, EMouseMove positiveMove) : m_MouseMovePair(negativeMove, positiveMove), m_IsKeyPair(false) {}

		bool IsKeyPair() const { m_IsKeyPair; }

		std::pair<EKey, EKey> GetKeyPair() const { return m_KeyPair; }
		std::pair<EMouseMove, EMouseMove> GetMouseMovePair() const { return m_MouseMovePair; }

	private:
		bool m_IsKeyPair;
		std::pair<EKey, EKey> m_KeyPair;
		std::pair<EMouseMove, EMouseMove> m_MouseMovePair;
	};
}