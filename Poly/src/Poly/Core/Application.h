#pragma once

#include "Window.h"
#include "LayerStack.h"

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
		bool m_running = true;
		Window* m_window;
		LayerStack m_layerStack;
	};

	Application* CreateApplication();

}