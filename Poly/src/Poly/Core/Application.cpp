#include "polypch.h"

#include "Poly/Core/Application.h"

#include "Poly/Events/WindowEvent.h"
#include "Poly/Rendering/Renderer.h"
#include "Poly/Core/Window.h"

namespace Poly
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		s_Instance = this;
	}

	void Application::Init()
	{
		m_pRenderer = Renderer::Create();

		if (std::optional<Window::Properties> windowProps = GetWindowProperties())
		{
			m_pWindow = Window::Create(windowProps.value());
			m_pRenderer->AddWindow(m_pWindow.get());
			m_pWindow->SetEventCallback([this](Event& event) { OnEvent(event); });
		}

		OnInit();
	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	void Application::Update(Timestamp dt)
	{
		if (m_pWindow)
			m_pWindow->Update();

		for (auto layer : m_LayerStack)
			layer->OnUpdate(dt);

		if (m_pRenderer)
			m_pRenderer->Render();
	}

	void Application::FixedUpdate(Timestamp dt)
	{

	}

	void Application::PushLayer(Layer* layer)
	{
		m_LayerStack.pushLayer(layer);
		layer->OnAttach();
	}

	void Application::PushOverlay(Layer* layer)
	{
		m_LayerStack.pushOverlay(layer);
		layer->OnAttach();
	}

	bool Application::IsRunning()
	{
		return m_Running;
	}

	Window* Application::GetWindow() const
	{
		return m_pWindow.get();
	}

	Renderer* Application::GetRenderer() const
	{
		return m_pRenderer.get();
	}

	void Application::OnEvent(Event& event)
	{
		EventDispatcher dispatcher(event);
		dispatcher.Dispatch<Events::WindowClosed>([this](Events::WindowClosed&) { m_Running = false; return false; });

		if (m_pRenderer)
			m_pRenderer->OnEvent(event);

		for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
		{
			if (event.Handled)
				break;

			(*it)->OnEvent(event);
		}
	}
}