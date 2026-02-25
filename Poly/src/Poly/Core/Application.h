#pragma once

#include "Poly/Core/LayerStack.h"
#include "Poly/Events/EventBus.h"
#include "Poly/Core/Timestamp.h"
#include "Poly/Core/Window.h"

#include <optional>

namespace Poly
{
	class Renderer;

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
		Renderer* GetRenderer() const;

	protected:
		virtual void OnInit() = 0;

		virtual std::optional<Window::Properties> GetWindowProperties() const { return std::nullopt; }

	private:
		static Application* s_Instance;

		bool m_Running = true;
		LayerStack m_LayerStack;
		Unique<Window> m_pWindow;
		Unique<Renderer> m_pRenderer;

		void OnCloseWindowEvent(CloseWindowEvent* e);
	};

	Application* CreateApplication();

}