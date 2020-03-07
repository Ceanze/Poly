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

		void run();

		void pushLayer(Layer* layer);
		void pushOverlay(Layer* layer);
	private:
		bool running = true;
		LayerStack layerStack;

		void onCloseWindowEvent(CloseWindowEvent* e);
	};

	Application* CreateApplication();

}