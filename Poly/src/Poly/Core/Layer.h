#pragma once

#include "Timestamp.h"

namespace Poly {
	class Event;

	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnEvent(Event& event) {};
		virtual void OnUpdate(Timestamp dt) {};
	};

}