#pragma once

#include "Poly/Core/Layer.h"

namespace Poly
{
	namespace Events
	{
		class MouseMoved;
		class MouseButtonPressed;
		class MouseButtonReleased;
		class MouseScrolled;
		class KeyPressed;
		class KeyReleased;
		class WindowResized;
	} // namespace Events

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void BeginFrame();

		void OnEvent(Event& event) override;

	private:
		bool OnMouseMoved(Events::MouseMoved& event);
		bool OnMouseButtonPressed(Events::MouseButtonPressed& event);
		bool OnMouseButtonReleased(Events::MouseButtonReleased& event);
		bool OnMouseScrolled(Events::MouseScrolled& event);
		bool OnKeyPressed(Events::KeyPressed& event);
		bool OnKeyReleased(Events::KeyReleased& event);
		bool OnWindowResized(Events::WindowResized& event);
	};
} // namespace Poly