#include "polypch.h"
#include "Application.h"
#include "Poly/Events/EventBus.h"
#include "Window.h"

namespace Poly
{
	Application* Application::s_Instance = nullptr;

	Application::Application()
	{
		POLY_EVENT_SUB(Application, OnCloseWindowEvent);

		s_Instance = this;
	}

	Application::~Application()
	{
		POLY_EVENT_UNSUB(Application, OnCloseWindowEvent);
	}

	void Application::Init()
	{
		if (std::optional<Window::Properties> windowProps = GetWindowProperties())
		{
			m_pWindow = Window::Create(windowProps.value());
		}

		OnInit();
	}

	Application& Application::Get()
	{
		return *s_Instance;
	}

	void Application::Update(Timestamp dt)
	{
		m_pWindow->Update();

		for (auto layer : m_LayerStack)
			layer->OnUpdate(dt);
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

	void Application::OnCloseWindowEvent(CloseWindowEvent* e)
	{
		m_Running = false;
	}
}