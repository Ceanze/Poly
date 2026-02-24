#pragma once

#include "LayerStack.h"
#include "Poly/Events/EventBus.h"
#include "Timestamp.h"
#include "Poly/Core/Window.h"

#include <optional>

namespace Poly
{
	class Application
	{
	public:
		Application();
		virtual ~Application();

		void Init();
		
		static Application& Get();

		void Update(Timestamp dt);
		void FixedUpdate(Timestamp dt);

		void PushLayer(Layer* layer);
		void PushOverlay(Layer* layer);

		bool IsRunning();

		Window* GetWindow() const;

	protected:
		virtual void OnInit() = 0;

		virtual std::optional<Window::Properties> GetWindowProperties() const { return std::nullopt; }

	private:
		static Application* s_Instance;

		bool m_Running = true;
		LayerStack m_LayerStack;
		Unique<Window> m_pWindow;

		void OnCloseWindowEvent(CloseWindowEvent* e);
	};

	Application* CreateApplication();

}