#pragma once

#include "Window.h"
#include "LayerStack.h"
#include "Poly/Events/EventBus.h"
#include "Timestamp.h"

namespace Poly {

	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Update(Timestamp dt);
		void FixedUpdate(Timestamp dt);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		bool IsRunning();

	private:
		bool m_Running = true;
		LayerStack m_LayerStack;

		void OnCloseWindowEvent(CloseWindowEvent* e);
	};

	Application* CreateApplication();

}