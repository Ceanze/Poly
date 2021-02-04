#include "polypch.h"
#include "Application.h"
#include "Poly/Events/EventBus.h"
#include "Window.h"
#include "RendererAPI.h"

#include <GLFW/glfw3.h>

namespace Poly
{

	Application::Application()
	{
		POLY_EVENT_SUB(Application, OnCloseWindowEvent);
	}

	Application::~Application()
	{
		POLY_EVENT_UNSUB(Application, OnCloseWindowEvent);
	}

	void Application::Update(Timestamp dt)
	{
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

	void Application::OnCloseWindowEvent(CloseWindowEvent* e)
	{
		m_Running = false;
	}
}