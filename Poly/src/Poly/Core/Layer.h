#pragma once

#include "Timestamp.h"

namespace Poly {

	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void OnAttach() {};
		virtual void OnDetach() {};
		virtual void OnUpdate(Timestamp dt) {};
	};

}