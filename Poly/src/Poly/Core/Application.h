#pragma once

#include "Window.h"
#include "LayerStack.h"
#include "Poly/Events/EventBus.h"

namespace Poly {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Run();

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);
	private:
		bool running = true;
		LayerStack layerStack;

		void OnCloseWindowEvent(CloseWindowEvent* e);
	};

	Application* CreateApplication();

}