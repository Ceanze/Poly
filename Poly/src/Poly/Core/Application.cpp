#include "polypch.h"
#include "Application.h"

namespace Poly
{

	Application::Application()
	{
		m_window = new Window(800, 300, "Poly");
		m_window->createConsole();

		Poly::Logger::init();

		POLY_CORE_INFO("Application created!");
	}

	Application::~Application()
	{

	}

	void Application::run()
	{
		while (m_running)
		{
			m_window->processMessages();

			for (auto layer : m_layerStack)
				layer->onUpdate();
		}
	}

	void Application::pushLayer(Layer* layer)
	{
		EventBus::get().publish(&TestEvent(32));

		m_layerStack.pushLayer(layer);
		layer->onAttach();
	}

	void Application::pushOverlay(Layer* layer)
	{
		m_layerStack.pushOverlay(layer);
		layer->onAttach();
	}
}