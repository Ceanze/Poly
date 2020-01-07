#pragma once

namespace Poly {

	class Layer
	{
	public:
		Layer() = default;
		virtual ~Layer() = default;

		virtual void onAttach() {};
		virtual void onDetach() {};
		virtual void onUpdate() {};
	};

}